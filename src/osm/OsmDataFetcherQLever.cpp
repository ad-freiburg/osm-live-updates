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

#include "osm/OsmDataFetcherQLever.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <strstream>
#include <spanstream>
#include <util/XmlHelper.h>

#include "simdjson.h"
#include "boost/regex.hpp"

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"
#include "osm/RelationMember.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
simdjson::padded_string
olu::osm::OsmDataFetcherQLever::OsmDataFetcherQLever::runQuery(const std::string &query,
                                                   const std::vector<std::string> &prefixes) {
    _sparqlWrapper.setQuery(query);
    _sparqlWrapper.setPrefixes(prefixes);
    return {_sparqlWrapper.runQuery()};
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Node>
olu::osm::OsmDataFetcherQLever::OsmDataFetcherQLever::fetchNodes(const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForNodeLocations(nodeIds),
        cnst::PREFIXES_FOR_NODE_LOCATION);

    std::vector<Node> nodes;
    nodes.reserve(nodeIds.size());

    forResults(response, [&nodes](simdjson::ondemand::value results) {
        auto it = results.begin();
        const auto nodeUri = getValue<std::string_view>((*it).value());
        ++it;
        const auto nodeLocationAsWkt = getValue<std::string_view>((*it).value());
        nodes.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri), wktPoint_t(nodeLocationAsWkt));
    });

    if (nodes.size() > nodeIds.size()) {
        std::cerr << "The SPARQL endpoint returned " << std::to_string(nodes.size())
                  << " locations, for " << std::to_string(nodeIds.size())
                  << " nodes." << std::endl;
        throw OsmDataFetcherException("Exception while trying to fetch nodes locations");
    }

    return nodes;
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherQLever::OsmDataFetcherQLever::fetchLatestTimestampOfAnyNode() {
    const auto response = runQuery(
        _queryWriter.writeQueryForLatestNodeTimestamp(),
        cnst::PREFIXES_FOR_LATEST_NODE_TIMESTAMP);

    std::string timestamp;
    const std::string timestampPath = "$.res[0][0]";
    if (const auto error = _parser.iterate(response).at_path(timestampPath).get_string(timestamp);
        error || timestamp.empty()) {
        std::cerr << "JSON error: " << error << std::endl;
        throw OsmDataFetcherException("Could not parse latest timestamp of any node from "
                                      "sparql endpoint");
    }

    if (_config.isQLever) {
        // QLever will return the timestamp in a format like:
        // "\"2025-05-24T19:15:22\"^^<http://www.w3.org/2001/XMLSchema#dateTime>"
        const boost::regex expr("\\\"([^\\\"]+)\\\"");
        if (boost::smatch match; regex_search(timestamp, match, expr)) {
            timestamp = match[1];
        } else {
            const std::string msg = "Could not extract timestamp from QLever"
                                    " response: " + timestamp;
            throw OsmDataFetcherException(msg.c_str());
        }
    }

    return timestamp;
}

namespace olu::osm {
    // _____________________________________________________________________________________________
    std::vector<Relation>
    OsmDataFetcherQLever::fetchRelations(const std::set<id_t> &relationIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelations(relationIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<Relation> relations;
        relations.reserve(relationIds.size());

        forResults(response, [&relations, this](simdjson::ondemand::value results) {
            auto it = results.begin();
            auto relationUri = getValue<std::string_view>((*it).value());
            auto relationId = OsmObjectHelper::parseIdFromUri(relationUri);
            Relation relation(relationId);

            ++it;
            auto relationType = getValue<std::string>((*it).value());
            relation.setType(util::XmlHelper::parseRdfString<std::string>(relationType));

            // Extract members for the relation
            ++it;
            auto memberUriList = getValue<std::string_view>((*it).value());
            memberUriList = memberUriList.substr(1, memberUriList.size() - 2);
            std::ispanstream uriStream(memberUriList);
            ++it;
            auto memberRolesList = getValue<std::string_view>((*it).value());
            memberRolesList = memberRolesList.substr(1, memberRolesList.size() - 2);
            std::ispanstream rolesStream(memberRolesList);
            ++it;
            auto memberPosList = getValue<std::string_view>((*it).value());
            memberPosList = memberPosList.substr(1, memberPosList.size() - 2);
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
        });

        return relations;
    }

    // _____________________________________________________________________________________________
    std::vector<Way> OsmDataFetcherQLever::fetchWays(const std::set<id_t> &wayIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForWaysMembers(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<Way> ways;
        ways.reserve(wayIds.size());

        forResults(response, [&ways, this](simdjson::ondemand::value results) {
            auto it = results.begin();
            auto wayUri = getValue<std::string_view>((*it).value());
            Way way(OsmObjectHelper::parseIdFromUri(wayUri));

            ++it;
            auto memberUriList = getValue<std::string_view>((*it).value());
            memberUriList = memberUriList.substr(1, memberUriList.size() - 2);
            std::ispanstream uriStream(memberUriList);
            ++it;
            auto memberPosList = getValue<std::string_view>((*it).value());
            memberPosList = memberPosList.substr(1, memberPosList.size() - 2);
            std::ispanstream posStream(memberPosList);

            std::string uri;
            std::string position;
            std::map<int, id_t> members;
            while (std::getline(uriStream, uri, ';')) {
                if (uri.empty()) { continue; }

                std::getline(posStream, position, ';');
                id_t memberId = OsmObjectHelper::parseIdFromUri(uri);
                members.emplace(std::stoi(position), memberId);
            }

            for (const auto &member : members | std::views::values) {
                way.addMember(member);
            }

            ways.emplace_back(way);
        });

        return ways;
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcherQLever::fetchWayInfos(Way &way) {
        const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_WAY,
                                                           std::to_string(way.getId()));
        const auto response = runQuery(
            _queryWriter.writeQueryForTagsAndMetaInfo(subject),
            cnst::PREFIXES_FOR_WAY_TAGS_AND_META_INFO);

        forResults(response, [&way, this](simdjson::ondemand::value results) {
            size_t counter = 0;
            std::string key;
            for (auto result: results) {
                if (result.is_null()) {
                    counter++;
                    continue;
                }

                switch (counter) {
                    case 0: {
                        key = getValue<std::string>(result.value());
                        break;
                    }
                    case 1: {
                        auto value = getValue<std::string>(result.value());
                        way.addTag(util::XmlHelper::parseKeyName(key),
                                   util::XmlHelper::parseRdfString<std::string>(value));
                        break;
                    }
                    case 2: {
                        auto timestamp = getValue<std::string>(result.value());
                        way.setTimestamp(util::XmlHelper::parseRdfString<std::string>(timestamp));
                        break;
                    }
                    case 3: {
                        const auto version = getValue<std::string>(result.value());
                        way.setVersion(util::XmlHelper::parseRdfString<int>(version));
                        break;
                    }
                    case 4: {
                        const auto changesetId = getValue<std::string>(result.value());
                        way.setChangesetId(util::XmlHelper::parseRdfString<int>(changesetId));
                        break;
                    }
                    default:
                        const std::string msg = "Cannot parse way info: "
                                                + std::string(result.raw_json().value());
                        throw OsmDataFetcherException(msg.c_str());
                }

                counter++;
            }
        });
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcherQLever::fetchRelationInfos(Relation &relation) {
        const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_REL,
                                                           std::to_string(relation.getId()));
        const auto response = runQuery(
            _queryWriter.writeQueryForTagsAndMetaInfo(subject),
            cnst::PREFIXES_FOR_RELATION_TAGS_AND_META_INFO);

        forResults(response, [&relation, this](simdjson::ondemand::value results) {
            size_t counter = 0;
            std::string key;
            for (auto result: results) {
                if (result.is_null()) {
                    counter++;
                    continue;
                }

                switch (counter) {
                    case 0: {
                        key = getValue<std::string>(result.value());
                        break;
                    }
                    case 1: {
                        auto value = getValue<std::string>(result.value());
                        relation.addTag(util::XmlHelper::parseKeyName(key),
                                        util::XmlHelper::parseRdfString<std::string>(value));
                        break;
                    }
                    case 2: {
                        auto timestamp = getValue<std::string>(result.value());
                        relation.setTimestamp(util::XmlHelper::parseRdfString<std::string>(timestamp));
                        break;
                    }
                    case 3: {
                        const auto version = getValue<std::string>(result.value());
                        relation.setVersion(util::XmlHelper::parseRdfString<int>(version));
                        break;
                    }
                    case 4: {
                        const auto changesetId = getValue<std::string>(result.value());
                        relation.setChangesetId(util::XmlHelper::parseRdfString<int>(changesetId));
                        break;
                    }
                    default:
                        const std::string msg = "Cannot parse way info: "
                                                + std::string(result.raw_json().value());
                        throw OsmDataFetcherException(msg.c_str());
                }

                counter++;
            }
        });
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcherQLever::fetchWaysMembers(const std::set<id_t> &wayIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForReferencedNodes(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<id_t> nodeIds;
        forResults(response, [&nodeIds, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto nodeUri = getValue<std::string_view>(result.value());
                nodeIds.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri));
            }
        });

        return nodeIds;
    }

// _________________________________________________________________________________________________
std::vector<std::pair<id_t, member_ids_t>>
olu::osm::OsmDataFetcherQLever::fetchWaysMembersSorted(const std::set<id_t> &wayIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForWaysMembers(wayIds),
        cnst::PREFIXES_FOR_WAY_MEMBERS);

    std::vector<std::pair<id_t, member_ids_t>> waysWithMembers;
    waysWithMembers.reserve(wayIds.size());

    forResults(response, [&waysWithMembers, this](simdjson::ondemand::value results) {
        auto it = results.begin();
        const auto wayUri = getValue<std::string_view>((*it).value());
        id_t wayId = OsmObjectHelper::parseIdFromUri(wayUri);

        ++it;
        const auto memberUriList = getValue<std::string_view>((*it).value());
        member_ids_t memberIds = parseValueList<id_t>(memberUriList,
                                                      [](const std::string &uri) {
                                                          return
                                                                  OsmObjectHelper::parseIdFromUri(
                                                                      uri);
                                                      });

        ++it;
        auto memberPositionsList = getValue<std::string_view>((*it).value());
        memberPositionsList = memberPositionsList.substr(1, memberPositionsList.size() - 2);
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
    });

    return waysWithMembers;
}

    // _____________________________________________________________________________________________
    std::vector<std::pair<id_t, std::vector<RelationMember>>>
    OsmDataFetcherQLever::fetchRelsMembersSorted(const std::set<id_t> &relIds) {
        const auto response = runQuery(_queryWriter.writeQueryForRelsMembers(relIds),
                                       cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<std::pair<id_t, std::vector<RelationMember>>> relsWithMembers;
        relsWithMembers.reserve(relIds.size());

        forResults(response, [&relsWithMembers, this](simdjson::ondemand::value results) {
            auto it = results.begin();
            auto relUri = getValue<std::string_view>((*it).value());
            id_t relId = OsmObjectHelper::parseIdFromUri(relUri);

            ++it;
            auto memberUriList = getValue<std::string_view>((*it).value());
            memberUriList = memberUriList.substr(1, memberUriList.size() - 2);
            member_ids_t memberIds = parseValueList<id_t>(memberUriList,
                                                          [](const std::string &uri) {
                                                              return OsmObjectHelper::parseIdFromUri(uri);
                                                          });
            std::vector<OsmObjectType> memberTypes = parseValueList<OsmObjectType>(memberUriList,
                [](const std::string &uri) {
                    return OsmObjectHelper::parseOsmTypeFromUri(uri);
                });

            ++it;
            auto memberPositionsList = getValue<std::string_view>((*it).value());
            memberPositionsList = memberPositionsList.substr(1, memberPositionsList.size() - 2);
            std::vector<int> memberPositions = parseValueList<int>(memberPositionsList,
                [](const std::string &pos) {
                    return stoi(pos);
                });

            ++it;
            auto memberRolesList = getValue<std::string_view>((*it).value());
            memberRolesList = memberRolesList.substr(1, memberRolesList.size() - 2);
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
        });

        return relsWithMembers;
    }

    // _____________________________________________________________________________________________
    std::pair<std::vector<id_t>, std::vector<id_t>>
    OsmDataFetcherQLever::fetchRelationMembers(const std::set<id_t> &relIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelationMemberIds(relIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<id_t> nodeIds;
        std::vector<id_t> wayIds;
        forResults(response, [&nodeIds, &wayIds, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto memberUri = getValue<std::string_view>(result.value());
                id_t memberId = OsmObjectHelper::parseIdFromUri(memberUri);
                if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
                    nodeIds.emplace_back(memberId);
                } else if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
                    wayIds.emplace_back(memberId);
                }
            }
        });

        return { nodeIds, wayIds };
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcherQLever::fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForWaysReferencingNodes(nodeIds),
            cnst::PREFIXES_FOR_WAYS_REFERENCING_NODE);

        std::vector<id_t> memberSubjects;
        forResults(response, [&memberSubjects, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto memberUri = getValue<std::string_view>(result.value());
                memberSubjects.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
            }
        });

        return memberSubjects;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t>
    OsmDataFetcherQLever::fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingNodes(nodeIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_NODE);

        std::vector<id_t> relationIds;
        forResults(response, [&relationIds, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto memberUri = getValue<std::string_view>(result.value());
                relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
            }
        });

        return relationIds;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcherQLever::fetchRelationsReferencingWays(const std::set<id_t> &wayIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingWays(wayIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_WAY);

        std::vector<id_t> relationIds;
        forResults(response, [&relationIds, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto memberUri = getValue<std::string_view>(result.value());
                relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
            }
        });

        return relationIds;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t>
    OsmDataFetcherQLever::fetchRelationsReferencingRelations(const std::set<id_t> &relationIds) {
        const auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingRelations(relationIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS);

        std::vector<id_t> refRelIds;
        forResults(response, [&refRelIds, this](simdjson::ondemand::value results) {
            for (auto result: results) {
                auto memberUri = getValue<std::string_view>(result.value());
                refRelIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
            }
        });

        return refRelIds;
    }
}

// _________________________________________________________________________________________________
template <typename T> std::vector<T>
olu::osm::OsmDataFetcherQLever::parseValueList(const std::string_view &list,
                                         const std::function<T(std::string)> function) {
    std::vector<T> items;
    std::ispanstream stream(list);

    std::string token;
    while (std::getline(stream, token, ';')) {
        items.push_back(function(token));
    }

    return items;
}

void
olu::osm::OsmDataFetcherQLever::forResults(const simdjson::padded_string &response,
                                           const std::function<void(simdjson::ondemand::value)> func) {
    auto doc = _parser.iterate(response);
    if (doc.error()) {
        std::cerr << doc.error() << std::endl;
        throw OsmDataFetcherException("Error while parsing response document.");
    }

    for (auto result : getResults(doc.value())) {
        if (result.error()) {
            std::cerr << result.error() << std::endl;
            throw OsmDataFetcherException("Error while parsing results.");
        }

        func(result.value());
    }
}

// _________________________________________________________________________________________________
simdjson::ondemand::value
olu::osm::OsmDataFetcherQLever::getResults(simdjson::ondemand::document &doc) {
    auto results = doc[cnst::KEY_QLEVER_RESULTS];
    if (results.error()) {
        std::cerr << results.error() << std::endl;
        throw OsmDataFetcherException("Error while getting results from document.");
    }

    return results.value();
}

// _________________________________________________________________________________________________
template <typename T> T
olu::osm::OsmDataFetcherQLever::getValue(simdjson::ondemand::value value) {
    try {
        if constexpr (std::is_same_v<T, std::string_view>) {
            return value.get_string();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string(value.get_string().value());
        } else if constexpr (std::is_same_v<T, int>) {
            const auto intString = std::string(value.get_string().value());
            return std::stoi(intString);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Cannot get value for results: "
                                + std::string(value.raw_json().value());
        throw OsmDataFetcherException(msg.c_str());
    }

    throw OsmDataFetcherException("The type of the value is not supported atm.");
}
