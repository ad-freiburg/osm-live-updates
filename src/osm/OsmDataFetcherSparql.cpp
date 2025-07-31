// Copyright 2025, University of Freiburg
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

#include "osm/OsmDataFetcherSparql.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <strstream>
#include <spanstream>
#include <util/XmlHelper.h>

#include "simdjson.h"

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"
#include "osm/RelationMember.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
simdjson::padded_string
olu::osm::OsmDataFetcherSparql::runQuery(const std::string &query,
                                                               const std::vector<std::string> &prefixes) {
    _stats->countQuery();

    _sparqlWrapper.setQuery(query);
    _sparqlWrapper.setPrefixes(prefixes);
    return {_sparqlWrapper.runQuery()};
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Node>
olu::osm::OsmDataFetcherSparql::fetchNodes(const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForNodeLocations(nodeIds),
        cnst::PREFIXES_FOR_NODE_LOCATION);

    std::vector<Node> nodes;
    nodes.reserve(nodeIds.size());

    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
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
void olu::osm::OsmDataFetcherSparql::fetchAndWriteNodesToFile(const std::string &filePath,
                                                              const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForNodeLocations(nodeIds),
        cnst::PREFIXES_FOR_NODE_LOCATION);

    std::ofstream outputFile;
    outputFile.open (filePath, std::ios::app);
    outputFile.precision(config::Config::DEFAULT_WKT_PRECISION);
    outputFile << std::fixed;

    size_t returnedNodeCount = 0;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        returnedNodeCount++;
        auto nodeUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        auto nodeLocationAsWkt = getValue<std::string_view>(binding[cnst::NAME_LOCATION]);
        const auto nodeId = OsmObjectHelper::parseIdFromUri(nodeUri);
        const auto nodeLocation = OsmObjectHelper::parseLonLatFromWktPoint(nodeLocationAsWkt);

        outputFile << util::XmlHelper::getNodeDummy(nodeId, nodeLocation) << std::endl;
    }

    outputFile.close();

    if (returnedNodeCount > nodeIds.size()) {
        std::cerr << "The SPARQL endpoint returned " << std::to_string(returnedNodeCount)
                << " locations, for " << std::to_string(nodeIds.size())
                << " nodes." << std::endl;
        throw OsmDataFetcherException("Exception while trying to fetch nodes locations");
    }
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherSparql::fetchLatestTimestamp() {
    const auto response = runQuery(
        _queryWriter.writeQueryForLatestTimestamp(),
        cnst::PREFIXES_FOR_LATEST_TIMESTAMP);

    std::string timestamp;
    const auto error = _parser.iterate(response).at_path(
        "$.results.bindings[0].latestTimestamp.value").get_string(timestamp);
    if (error || timestamp.empty()) {
        std::cerr << "JSON error: " << error << std::endl;
        throw OsmDataFetcherException("Could not parse latest timestamp of any node from "
                                      "sparql endpoint");
    }

    return timestamp;
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Relation>
olu::osm::OsmDataFetcherSparql::fetchRelations(const std::set<id_t> &relationIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelations(relationIds),
        cnst::PREFIXES_FOR_RELATION_MEMBERS);

    std::vector<Relation> relations;
    relations.reserve(relationIds.size());

    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {

        // Set id and type of the relation
        auto relationUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        auto relationId = OsmObjectHelper::parseIdFromUri(relationUri);
        Relation relation(relationId);

        auto relationType = getValue<std::string>(binding[cnst::NAME_TYPE]);
        relation.setType(relationType);

        // Extract members for the relation
        auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
        auto memberRolesList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_ROLES]);
        auto memberPosList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);

        const auto members = OsmObjectHelper::parseRelationMemberList(memberUriList, memberRolesList,
                                                                                      memberPosList);
        for (const auto &member : members) {
            relation.addMember(member);
        }

        relations.emplace_back(relation);
    }

    return relations;
}

// _________________________________________________________________________________________________
size_t
olu::osm::OsmDataFetcherSparql::fetchAndWriteRelationsToFile(const std::string &filePath,
                                                             const std::set<id_t> &relationIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelations(relationIds),
        cnst::PREFIXES_FOR_RELATION_MEMBERS);

    std::ofstream outputFile;
    outputFile.open (filePath, std::ios::app);

    size_t returnedRelationsCount = 0;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {

        // Set id and type of the relation
        auto relationUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        auto relationId = OsmObjectHelper::parseIdFromUri(relationUri);
        auto relationType = getValue<std::string>(binding[cnst::NAME_TYPE]);

        // Extract members for the relation
        auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
        auto memberRolesList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_ROLES]);
        auto memberPosList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);
        const auto members = OsmObjectHelper::parseRelationMemberList(
                memberUriList, memberRolesList, memberPosList);

        // Write relation to file
        outputFile << util::XmlHelper::getRelationDummy(relationId, relationType, members)
                   << std::endl;
    }

    return returnedRelationsCount;
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Way>
olu::osm::OsmDataFetcherSparql::fetchWays(const std::set<id_t> &wayIds) {
    auto response = runQuery(
        _queryWriter.writeQueryForWaysMembers(wayIds),
        cnst::PREFIXES_FOR_WAY_MEMBERS);

    std::vector<Way> ways;
    ways.reserve(wayIds.size());

    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto wayUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        Way way(OsmObjectHelper::parseIdFromUri(wayUri));

        auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
        auto memberPosList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);

        // Extract way infos from response
        const auto members = OsmObjectHelper::parseWayMemberList(memberUriList, memberPosList);
        for (const auto &member : members) {
            way.addMember(member);
        }

        ways.emplace_back(way);
    }

    return ways;
}

// _________________________________________________________________________________________________
size_t olu::osm::OsmDataFetcherSparql::fetchAndWriteWaysToFile(const std::string &filePath,
                                                               const std::set<id_t> &wayIds) {
    auto response = runQuery(
            _queryWriter.writeQueryForWaysMembers(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

    std::ofstream outputFile;
    outputFile.open (filePath, std::ios::app);

    size_t returnedWayCount = 0;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        returnedWayCount++;

        auto wayUri = getValue<std::string_view>(binding[cnst::NAME_VALUE]);
        auto memberUriList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_IDS]);
        auto memberPosList = getValue<std::string_view>(binding[cnst::NAME_MEMBER_POSS]);

        // Extract way infos from response
        const auto wayId = OsmObjectHelper::parseIdFromUri(wayUri);
        auto members = OsmObjectHelper::parseWayMemberList(memberUriList, memberPosList);

        // Write way to file
        outputFile << util::XmlHelper::getWayDummy(wayId, members) << std::endl;
    }

    return returnedWayCount;
}

// _________________________________________________________________________________________________
void olu::osm::OsmDataFetcherSparql::fetchWayInfos(Way &way) {
    const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_WAY,
                                                       std::to_string(way.getId()));
    const auto response = runQuery(
        _queryWriter.writeQueryForTagsAndMetaInfo(subject),
        cnst::PREFIXES_FOR_WAY_TAGS_AND_META_INFO);

    for (auto doc = _parser.iterate(response); auto binding: getBindings(doc)) {
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

// _________________________________________________________________________________________________
void olu::osm::OsmDataFetcherSparql::fetchRelationInfos(Relation &relation) {
    const std::string subject = cnst::MakePrefixedName(cnst::NAMESPACE_OSM_REL,
                                                       std::to_string(relation.getId()));
    const auto response = runQuery(
        _queryWriter.writeQueryForTagsAndMetaInfo(subject),
        cnst::PREFIXES_FOR_RELATION_TAGS_AND_META_INFO);

    for (auto doc = _parser.iterate(response); auto binding: getBindings(doc)) {
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

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherSparql::fetchWaysMembers(const std::set<id_t> &wayIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForReferencedNodes(wayIds),
        cnst::PREFIXES_FOR_WAY_MEMBERS);

    std::vector<id_t> nodeIds;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto nodeUri = getValue<std::string_view>(binding[cnst::NAME_NODE]);
        nodeIds.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri));
    }

    return nodeIds;
}

// _________________________________________________________________________________________________
std::vector<std::pair<olu::id_t, olu::member_ids_t>>
olu::osm::OsmDataFetcherSparql::fetchWaysMembersSorted(const std::set<id_t> &wayIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForWaysMembers(wayIds),
        cnst::PREFIXES_FOR_WAY_MEMBERS);

    std::vector<std::pair<id_t, member_ids_t>> waysWithMembers;
    waysWithMembers.reserve(wayIds.size());

    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
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

// _________________________________________________________________________________________________
std::vector<std::pair<olu::id_t, std::vector<olu::osm::RelationMember>>>
olu::osm::OsmDataFetcherSparql::fetchRelsMembersSorted(const std::set<id_t> &relIds) {
    const auto response = runQuery(_queryWriter.writeQueryForRelsMembers(relIds),
                                   cnst::PREFIXES_FOR_RELATION_MEMBERS);

    std::vector<std::pair<id_t, std::vector<RelationMember>>> relsWithMembers;
    relsWithMembers.reserve(relIds.size());

    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
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

// _________________________________________________________________________________________________
std::pair<std::vector<olu::id_t>, std::vector<olu::id_t>>
olu::osm::OsmDataFetcherSparql::fetchRelationMembers(const std::set<id_t> &relIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelationMemberIds(relIds),
        cnst::PREFIXES_FOR_RELATION_MEMBERS);

    std::vector<id_t> nodeIds;
    std::vector<id_t> wayIds;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto memberUri = getValue<std::string_view>(binding[cnst::NAME_MEMBER]);

        id_t memberId = OsmObjectHelper::parseIdFromUri(memberUri);
        if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
            nodeIds.emplace_back(memberId);
        } else if (memberUri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
            wayIds.emplace_back(memberId);
        }
    }

    return { nodeIds, wayIds };
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherSparql::fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForWaysReferencingNodes(nodeIds),
        cnst::PREFIXES_FOR_WAYS_REFERENCING_NODE);

    std::vector<id_t> memberSubjects;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto memberUri = getValue<std::string_view>(binding[cnst::NAME_WAY]);
        memberSubjects.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
    }

    return memberSubjects;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherSparql::fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelationsReferencingNodes(nodeIds),
        cnst::PREFIXES_FOR_RELATIONS_REFERENCING_NODE);

    std::vector<id_t> relationIds;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
        relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
    }

    return relationIds;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherSparql::fetchRelationsReferencingWays(const std::set<id_t> &wayIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelationsReferencingWays(wayIds),
        cnst::PREFIXES_FOR_RELATIONS_REFERENCING_WAY);

    std::vector<id_t> relationIds;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
        relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
    }

    return relationIds;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherSparql::fetchRelationsReferencingRelations(const std::set<id_t> &relationIds) {
    const auto response = runQuery(
        _queryWriter.writeQueryForRelationsReferencingRelations(relationIds),
        cnst::PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS);

    std::vector<id_t> refRelIds;
    for (auto doc = _parser.iterate(response); auto binding : getBindings(doc)) {
        auto memberUri = getValue<std::string_view>(binding[cnst::NAME_REL]);
        refRelIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
    }

    return refRelIds;
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherSparql::fetchOsm2RdfVersion() {
    const auto response = runQuery(_queryWriter.writeQueryForOsm2RdfVersion(),
                             cnst::PREFIXES_FOR_OSM2RDF_VERSION);

    std::set<std::string> versions;
    for (auto doc = _parser.iterate(response);
        auto binding : getBindings(doc)) {
        auto version = getValue<std::string>(binding[cnst::NAME_VALUE]);
        versions.insert(util::XmlHelper::parseRdfString<std::string>(version));
    }

    if (versions.size() == 0) {
        throw OsmDataFetcherException("Could not fetch osm2rdf version from SPARQL endpoint.");
    }

    if (versions.size() > 1) {
        throw OsmDataFetcherException("SPARQL endpoint returned multiple different osm2rdf"
                                      " versions.");
    }

    return *versions.begin();
}

// _________________________________________________________________________________________________
std::map<std::string, std::string> olu::osm::OsmDataFetcherSparql::fetchOsm2RdfOptions() {
    const auto response = runQuery(_queryWriter.writeQueryForOsm2RdfOptions(),
                            cnst::PREFIXES_FOR_OSM2RDF_OPTIONS);

    std::map<std::string, std::string> options;
    for (auto doc = _parser.iterate(response);
        auto binding : getBindings(doc)) {
        const auto optionIRI =  getValue<std::string_view>(binding[cnst::NAME_OPTION]);
        const auto optionValue = getValue<std::string>(binding[cnst::NAME_VALUE]);

        options.insert_or_assign(OsmObjectHelper::parseOsm2rdfOptionName(optionIRI),
                                 util::XmlHelper::parseRdfString<std::string>(optionValue));
    }

    return options;
}

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState olu::osm::OsmDataFetcherSparql::fetchUpdatesCompleteUntil() {
    const auto response = runQuery(_queryWriter.writeQueryForUpdatesCompleteUntil(),
        cnst::PREFIXES_FOR_METADATA_TRIPLES);

    std::set<OsmDatabaseState> updatesCompleteUntilResponses;

    for (auto doc = _parser.iterate(response);
        auto binding : getBindings(doc)) {
         try {
            auto databaseState = getValue<std::string>(binding[cnst::NAME_UPDATES_COMPLETE_UNTIL]);
            updatesCompleteUntilResponses.insert(from_string(databaseState));
         } catch (std::exception &e) {
             util::Logger::log(util::LogEvent::WARNING,
                               "SPARQL endpoint returned invalid database state for "
                               "'osm2rdfmeta:updatesCompleteUntil' predicate: "
                               + std::string(e.what()));
         }
    }

    if (const auto maxIt = std::max_element(updatesCompleteUntilResponses.begin(),
                                            updatesCompleteUntilResponses.end());
        maxIt != updatesCompleteUntilResponses.end()) {
        const OsmDatabaseState &maxState = *maxIt;
        return maxState;
    }

    throw OsmDataFetcherException("SPARQL endpoint did not return a valid database state.");
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherSparql::fetchReplicationServer() {
    const auto response = runQuery(_queryWriter.writeQueryForReplicationServer(),
        cnst::PREFIXES_FOR_METADATA_TRIPLES);

    std::set<std::string> replicationServers;

    for (auto doc = _parser.iterate(response);
        auto binding : getBindings(doc)) {
        try {
            auto replicationServer = getValue<std::string>(binding[cnst::NAME_REPLICATION_SERVER]);
            replicationServers.insert(replicationServer);
        } catch (std::exception &e) {
            util::Logger::log(util::LogEvent::WARNING,
                              "SPARQL endpoint returned invalid replication server uri: "
                              + std::string(e.what()));
        }
    }

    if (replicationServers.size() > 1) {
        util::Logger::log(util::LogEvent::WARNING,
                "SPARQL endpoint returned multiple replication server uris");
    }

    return replicationServers.empty() ? "" : *replicationServers.begin();
}

// _________________________________________________________________________________________________
template <typename T> std::vector<T>
olu::osm::OsmDataFetcherSparql::parseValueList(const std::string_view &list,
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
simdjson::simdjson_result<simdjson::ondemand::value>
olu::osm::OsmDataFetcherSparql::getBindings(
    simdjson::simdjson_result<simdjson::ondemand::document> &doc) {
    return doc[config::constants::KEY_RESULTS][config::constants::KEY_BINDINGS];
}

// _________________________________________________________________________________________________
template <typename T> T
olu::osm::OsmDataFetcherSparql::getValue(
    simdjson::simdjson_result<simdjson::ondemand::value> value) {
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