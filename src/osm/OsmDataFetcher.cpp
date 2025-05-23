// Copyright 2024, University of Freiburg
// Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

// This file is part of osm-live-updates.
//
// osm-live-updates is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm-live-updates is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

#include "osm/OsmDataFetcher.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <strstream>
#include <spanstream>

#include "simdjson.h"

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"
#include "osm/RelationMember.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
simdjson::padded_string
olu::osm::OsmDataFetcher::OsmDataFetcher::runQuery(const std::string &query,
                                                   const std::vector<std::string> &prefixes) {
    _sparqlWrapper.setQuery(query);
    _sparqlWrapper.setPrefixes(prefixes);
    return simdjson::padded_string(_sparqlWrapper.runQuery());
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Node>
olu::osm::OsmDataFetcher::OsmDataFetcher::fetchNodes(const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForNodeLocations(nodeIds),
        cnst::PREFIXES_FOR_NODE_LOCATION);

    std::vector<Node> nodes;
    nodes.reserve(nodeIds.size());

    auto doc = _parser.iterate(response);
    for (auto binding : getBindings(doc)) {
        auto nodeUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        auto nodeLocationAsWkt = getValue<std::string_view>(binding[cnst::NAME_LOCATION]);
        nodes.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri), wktPoint_t(nodeLocationAsWkt));
    }

    if (nodes.size() > nodeIds.size()) {
        std::cerr << "The SPARQL endpoint returned " << std::to_string(nodes.size())
                  << " locations, for " << std::to_string(nodeIds.size())
                  << " nodes." << std::endl;
        throw OsmDataFetcherException("Exception while trying to fetch nodes locations");
    }

    return nodes;
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcher::OsmDataFetcher::fetchLatestTimestampOfAnyNode() {
    const auto response = runQuery(
        _queryWriter.writeQueryForLatestNodeTimestamp(),
        cnst::PREFIXES_FOR_LATEST_NODE_TIMESTAMP);

    std::string timestamp;
    const auto error = _parser.iterate(response).at_path(
        "$.results.bindings[0].timestamp.value").get_string(timestamp);
    if (error || timestamp.empty()) {
        std::cerr << "JSON error: " << error << std::endl;
        throw OsmDataFetcherException("Could not parse latest timestamp of any node from "
                                      "sparql endpoint");
    }

    return timestamp;
}

namespace olu::osm {
    // _____________________________________________________________________________________________
    std::vector<Relation>
    OsmDataFetcher::fetchRelations(const std::set<id_t> &relationIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelations(relationIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<Relation> relations;
        relations.reserve(relationIds.size());

        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {

            // Set id and type of the relation
            auto relationUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
            auto relationId = OsmObjectHelper::parseIdFromUri(relationUri);
            Relation relation(relationId);

            auto relationType = getValue<std::string>(binding[cnst::NAME_TYPE]);
            relation.setType(relationType);

            // Extract members for the relation
            auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
            std::ispanstream uriStream(memberUriList);
            auto memberRolesList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_ROLES]);
            std::ispanstream rolesStream(memberRolesList);
            auto memberPosList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);
            std::ispanstream posStream(memberPosList);

            std::string memberUri;
            std::string memberRole;
            std::string memberPositionString;
            std::map<int, RelationMember> members;
            while (std::getline(uriStream, memberUri, ';')) {
                if (memberUri.empty()) {
                    throw OsmDataFetcherException("Cannot parse member uri");
                }

                std::getline(rolesStream, memberRole, ';');
                std::getline(posStream, memberPositionString, ';');

                int memberPosition;
                try {
                    memberPosition = std::stoi(memberPositionString);
                } catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    const std::string msg = "Cannot parse member position: " + memberPositionString;
                    throw OsmDataFetcherException(msg.c_str());
                }

                members.emplace(memberPosition, RelationMember(memberUri, memberRole));
            }

            for (const auto &member : members | std::views::values) {
                relation.addMember(member);
            }

            relations.emplace_back(relation);
        }

        return relations;
    }

    // _____________________________________________________________________________________________
    std::vector<Way> OsmDataFetcher::fetchWays(const std::set<id_t> &wayIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForWaysMembers(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<Way> ways;
        ways.reserve(wayIds.size());

        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto wayUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
            Way way(OsmObjectHelper::parseIdFromUri(wayUri));

            std::ispanstream uriStream(
                getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]));
            std::ispanstream posStream(
                getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]));

            std::string uri;
            std::string position;
            std::map<int, id_t> members;
            while (std::getline(uriStream, uri, ';')) {
                if (uri.empty()) { continue; }

                std::getline(posStream, position, ';');
                id_t id = OsmObjectHelper::parseIdFromUri(uri);
                members.emplace(std::stoi(position), id);
            }

            for (const auto &member : members | std::views::values) {
                way.addMember(member);
            }

            ways.emplace_back(way);
        }

        return ways;
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcher::fetchWayInfos(Way &way) {
        const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_WAY,
                                                           std::to_string(way.getId()));
        const auto response = runQuery(
            _queryWriter.writeQueryForTagsAndMetaInfo(subject),
            cnst::PREFIXES_FOR_WAY_TAGS_AND_META_INFO);

        auto doc = _parser.iterate(response);
        for (auto binding: getBindings(doc)) {
            if (binding[cnst::NAME_KEY].error() == simdjson::SUCCESS) {
                auto key = getValue<std::string>(binding[cnst::NAME_KEY]);
                auto value = getValue<std::string>(binding[cnst::NAME_VALUE]);
                way.addTag(key, value);
            } else if (binding[cnst::NAME_TIMESTAMP].error() == simdjson::SUCCESS) {
                way.setTimestamp(getValue<std::string>(binding[cnst::NAME_TIMESTAMP]));
            } else if (binding[cnst::NAME_VERSION].error() == simdjson::SUCCESS) {
                way.setVersion(getValue<int>(binding[cnst::NAME_VERSION]));
            } else if (binding[cnst::NAME_CHANGESET].error() == simdjson::SUCCESS) {
                way.setChangesetId(getValue<int>(binding[cnst::NAME_CHANGESET]));
            } else {
                const std::string msg = "Cannot parse way info: "
                                        + std::string(binding.raw_json().value());
                throw OsmDataFetcherException(msg.c_str());
            }
        }
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcher::fetchRelationInfos(Relation &relation) {
        const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_REL,
                                                           std::to_string(relation.getId()));
        const auto response = runQuery(
            _queryWriter.writeQueryForTagsAndMetaInfo(subject),
            cnst::PREFIXES_FOR_RELATION_TAGS_AND_META_INFO);

        auto doc = _parser.iterate(response);
        for (auto binding: getBindings(doc)) {
            if (binding[cnst::NAME_KEY].error() == simdjson::SUCCESS) {
                auto key = getValue<std::string>(binding[cnst::NAME_KEY]);
                auto value = getValue<std::string>(binding[cnst::NAME_VALUE]);
                relation.addTag(key, value);
            } else if (binding[cnst::NAME_TIMESTAMP].error() == simdjson::SUCCESS) {
                relation.setTimestamp(getValue<std::string>(binding[cnst::NAME_TIMESTAMP]));
            } else if (binding[cnst::NAME_VERSION].error() == simdjson::SUCCESS) {
                relation.setVersion(getValue<int>(binding[cnst::NAME_VERSION]));
            } else if (binding[cnst::NAME_CHANGESET].error() == simdjson::SUCCESS) {
                relation.setChangesetId(getValue<int>(binding[cnst::NAME_CHANGESET]));
            } else {
                const std::string msg = "Cannot parse way info: "
                                        + std::string(binding.raw_json().value());
                throw OsmDataFetcherException(msg.c_str());
            }
        }
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcher::fetchWaysMembers(const std::set<id_t> &wayIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForReferencedNodes(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<id_t> nodeIds;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto nodeUri = getValue<std::string_view>(binding[cnst::NAME_NODE]);
            nodeIds.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri));
        }

        return nodeIds;
    }

    // _____________________________________________________________________________________________
    std::vector<std::pair<id_t, member_ids_t>>
    OsmDataFetcher::fetchWaysMembersSorted(const std::set<id_t> &wayIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForWaysMembers(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<std::pair<id_t, member_ids_t>> waysWithMembers;
        waysWithMembers.reserve(wayIds.size());

        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto wayUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
            id_t wayId = OsmObjectHelper::parseIdFromUri(wayUri);

            auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
            member_ids_t memberIds = parseValueList<id_t>(memberUriList,
                [](const std::string &uri) {
                    return OsmObjectHelper::parseIdFromUri(uri);
                });

            auto memberPositionsList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);
            std::vector<int> memberPositions = parseValueList<int>(memberPositionsList,
                [](const std::string &pos) {
                    return stoi(pos);
                });

            // The list of members that the sparql endpoint returns is not necessarily sorted, so
            // we have to sort them by their position
            std::vector<std::pair<int, int>> paired;
            for (size_t i = 0; i < memberIds.size(); ++i) {
                paired.emplace_back(memberPositions[i], memberIds[i]);
            }

            std::ranges::sort(paired);

            for (size_t i = 0; i < memberIds.size(); ++i) {
                memberIds[i] = paired[i].second;
            }

            waysWithMembers.emplace_back(wayId, memberIds);
        }

        return waysWithMembers;
    }

    // _____________________________________________________________________________________________
    std::vector<std::pair<id_t, std::vector<RelationMember>>>
    OsmDataFetcher::fetchRelsMembersSorted(const std::set<id_t> &relIds) {
        auto response = runQuery(_queryWriter.writeQueryForRelsMembers(relIds),
                                       cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<std::pair<id_t, std::vector<RelationMember>>> relsWithMembers;
        relsWithMembers.reserve(relIds.size());

        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto relUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
            id_t relId = OsmObjectHelper::parseIdFromUri(relUri);

            auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
            member_ids_t memberIds = parseValueList<id_t>(memberUriList,
                [](const std::string &uri) {
                    return OsmObjectHelper::parseIdFromUri(uri);
                });
            std::vector<OsmObjectType> memberTypes = parseValueList<OsmObjectType>(memberUriList,
                [](const std::string &uri) {
                    return OsmObjectHelper::parseOsmTypeFromUri(uri);
                });

            auto memberPositionsList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);
            std::vector<int> memberPositions = parseValueList<int>(memberPositionsList,
                [](const std::string &pos) {
                    return stoi(pos);
                });

            auto memberRolesList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);
            std::vector<std::string> memberRoles = parseValueList<std::string>(memberRolesList,
                [](const std::string &pos) {
                    return pos;
                });

            std::vector<std::pair<int, RelationMember>> paired;
            for (size_t i = 0; i < memberIds.size(); ++i) {
                paired.emplace_back(memberPositions[i],
                    RelationMember(memberIds[i], memberTypes[i], memberRoles[i]));
            }

            // Sort by position
            std::ranges::sort(paired, [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

            std::vector<RelationMember> sortedMembers;
            for (const auto &member: paired | std::views::values) {
                sortedMembers.push_back(member);
            }

            relsWithMembers.emplace_back(relId, sortedMembers);
        }

        return relsWithMembers;
    }

    // _____________________________________________________________________________________________
    std::pair<std::vector<id_t>, std::vector<id_t>>
    OsmDataFetcher::fetchRelationMembers(const std::set<id_t> &relIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationMemberIds(relIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<id_t> nodeIds;
        std::vector<id_t> wayIds;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto memberUri = getValue<std::string_view>(binding[cnst::NAME_MEMBER]);

            id_t id = OsmObjectHelper::parseIdFromUri(memberUri);
            if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
                nodeIds.emplace_back(id);
            } else if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
                wayIds.emplace_back(id);
            }
        }

        return { nodeIds, wayIds };
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcher::fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForWaysReferencingNodes(nodeIds),
            cnst::PREFIXES_FOR_WAYS_REFERENCING_NODE);

        std::vector<id_t> memberSubjects;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto memberUri = getValue<std::string_view>(binding[cnst::NAME_WAY]);
            memberSubjects.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
        }

        return memberSubjects;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t>
    OsmDataFetcher::fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingNodes(nodeIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_NODE);

        std::vector<id_t> relationIds;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
            relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
        }

        return relationIds;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcher::fetchRelationsReferencingWays(const std::set<id_t> &wayIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingWays(wayIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_WAY);

        std::vector<id_t> relationIds;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
            relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
        }

        return relationIds;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t>
    OsmDataFetcher::fetchRelationsReferencingRelations(const std::set<id_t> &relationIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingRelations(relationIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS);

        std::vector<id_t> refRelIds;
        auto doc = _parser.iterate(response);
        for (auto binding : getBindings(doc)) {
            auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
            refRelIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
        }

        return refRelIds;
    }
}

// _________________________________________________________________________________________________
template <typename T> std::vector<T>
olu::osm::OsmDataFetcher::parseValueList(const std::string_view &list,
                                         const std::function<T(std::string)> function) {
    std::vector<T> items;
    std::ispanstream stream(list);

    std::string token;
    while (std::getline(stream, token, ';')) {
        items.push_back(function(token));
    }

    return items;
}

// _________________________________________________________________________________________________
simdjson::simdjson_result<simdjson::westmere::ondemand::value>
olu::osm::OsmDataFetcher::getBindings(
    simdjson::simdjson_result<simdjson::ondemand::document> &doc) {
    return doc[config::constants::KEY_RESULTS][config::constants::KEY_BINDINGS];
}

// _________________________________________________________________________________________________
template <typename T> T
olu::osm::OsmDataFetcher::getValue(
    simdjson::simdjson_result<simdjson::westmere::ondemand::value> value) {
    try {
        if constexpr (std::is_same_v<T, std::string_view>) {
            return value[config::constants::KEY_VALUE].get_string();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string(value[config::constants::KEY_VALUE].get_string().value());
        } else if constexpr (std::is_same_v<T, int>) {
            const auto intString = std::string(
                value[config::constants::KEY_VALUE].get_string().value());
            return std::stoi(intString);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Cannot get value for binding: "
                                + std::string(value.raw_json().value());
        throw OsmDataFetcherException(msg.c_str());
    }

    throw OsmDataFetcherException("The type of the value is not supported atm.");
}