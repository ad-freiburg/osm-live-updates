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
#include <regex>

#include "simdjson.h"

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"
#include "osm/RelationMember.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"
#include "util/XmlHelper.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::osm::OsmDataFetcherQLever::runQuery(const std::string &query,
                                              const std::vector<std::string> &prefixes,
                                              std::function<void(simdjson::ondemand::value)>
                                              resultFunc) {
    _stats->countQuery();

    _sparqlWrapper.setQuery(query);
    _sparqlWrapper.setPrefixes(prefixes);

    auto response = _sparqlWrapper.runQuery();
    auto paddedResponse = simdjson::padded_string(response);
    for (auto doc = _parser.iterate(paddedResponse);
         auto field: doc.get_object()) {
        if (field.error()) {
            std::cerr << field.error() << std::endl;
            throw OsmDataFetcherException("Error while parsing QLever response.");
        }

        const std::string_view key = field.escaped_key();
        if (key == cnst::KEY_QLEVER_RESULTS) {
            for (auto result: field.value()) {
                resultFunc(result.value());
            }
        }
        if (key == cnst::KEY_QLEVER_TIME) {
            _stats->logQleverQueryInfo(field.value().get_object());
        }
    }

    // Write SPARQL response to a file, if configured by the user
    if (!_config.sparqlResponseFile.empty()) {
        std::ofstream outputFile(_config.sparqlResponseFile, std::ios::app);
        if (!outputFile) {
            std::cerr << "Error opening file for SPARQL response output." << std::endl;
            throw OsmDataFetcherException("Cannot open file for SPARQL response output.");
        }
        outputFile << response << std::endl;
        outputFile.close();
    }
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherQLever::fetchLatestTimestamp() {
    std::string timestamp;
    runQuery(_queryWriter.writeQueryForLatestTimestamp(),
             cnst::PREFIXES_FOR_LATEST_TIMESTAMP,
             [&timestamp](simdjson::ondemand::value results) {
                 for (auto value: results) {
                     // QLever will return the timestamp in rdf syntax, e.g.:
                     // "\"2025-05-24T19:15:22\"^^<http://www.w3.org/2001/XMLSchema#dateTime>"
                     auto response = getValue<std::string>(value.value());
                     const std::regex expr("\\\"([^\\\"]+)\\\"");
                     if (std::smatch match; regex_search(response, match, expr)) {
                         timestamp = match[1];
                     } else {
                         const std::string msg = "Could not extract timestamp from QLever"
                                                 " response: " + response;
                         throw OsmDataFetcherException(msg.c_str());
                     }
                 }
             });

    if (timestamp.empty()) {
        throw OsmDataFetcherException("Could not extract timestamp from QLever");
    }

    return timestamp;
}

// _________________________________________________________________________________________________
std::vector<olu::osm::Node>
olu::osm::OsmDataFetcherQLever::fetchNodes(const std::set<id_t> &nodeIds) {
    std::vector<Node> nodes;
    nodes.reserve(nodeIds.size());

    runQuery(_queryWriter.writeQueryForNodeLocations(nodeIds), cnst::PREFIXES_FOR_NODE_LOCATION,
             [&nodes](simdjson::ondemand::value results) {
                 auto it = results.begin();
                 const auto nodeUri = getValue<std::string_view>((*it).value());
                 ++it;
                 const auto nodeLocationAsWkt = getValue<std::string_view>((*it).value());
                 nodes.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri),
                                    wktPoint_t(nodeLocationAsWkt));
             });

    if (nodes.size() > nodeIds.size()) {
        util::Logger::log(util::LogEvent::WARNING,
                          "The SPARQL endpoint returned multiple locations for one or more nodes.");
    }

    return nodes;
}

// _________________________________________________________________________________________________
void olu::osm::OsmDataFetcherQLever::fetchAndWriteNodesToFile(const std::string &filePath, const std::set<id_t> &nodeIds) {
    std::ofstream outputFile;
    outputFile.open(filePath, std::ios::app);
    outputFile.precision(config::Config::DEFAULT_WKT_PRECISION);
    outputFile << std::fixed;

    size_t returnedNodeCount = 0;
    runQuery(_queryWriter.writeQueryForNodeLocations(nodeIds), cnst::PREFIXES_FOR_NODE_LOCATION,
             [&returnedNodeCount, &outputFile](simdjson::ondemand::value results) {
                 returnedNodeCount++;

                 auto it = results.begin();
                 const auto nodeUri = getValue<std::string_view>((*it).value());
                 ++it;
                 const auto nodeLocationAsWkt = getValue<std::string_view>((*it).value());

                 const auto nodeId = OsmObjectHelper::parseIdFromUri(nodeUri);
                 const auto nodeLocation = OsmObjectHelper::parseLonLatFromWktPoint(
                     nodeLocationAsWkt);
                 const auto nodeXml = util::XmlHelper::getNodeDummy(nodeId, nodeLocation);
                 outputFile.write(nodeXml.data(), nodeXml.size());
                 outputFile << std::endl;
             });

    outputFile.close();

    if (returnedNodeCount > nodeIds.size()) {
        std::cerr << "The SPARQL endpoint returned " << std::to_string(returnedNodeCount)
                << " locations, for " << std::to_string(nodeIds.size())
                << " nodes." << std::endl;
        throw OsmDataFetcherException("Exception while trying to fetch nodes locations");
    }
}

// _________________________________________________________________________________________________
size_t
olu::osm::OsmDataFetcherQLever::fetchAndWriteRelationsToFile(const std::string &filePath,
                                                             const std::set<id_t> &relationIds) {
    std::ofstream outputFile;
    outputFile.open (filePath, std::ios::app);

    size_t returnedRelationsCount = 0;
    runQuery(_queryWriter.writeQueryForRelations(relationIds), cnst::PREFIXES_FOR_RELATION_MEMBERS,
             [&outputFile, &returnedRelationsCount, this](simdjson::ondemand::value results) {
                 returnedRelationsCount++;

                 auto it = results.begin();
                 const auto relationUri = getValue<std::string_view>((*it).value());

                 ++it;
                 auto relationType = getValue<std::string_view>((*it).value());
                 // Remove the surrounding quotes from the relation type
                 relationType = relationType.substr(1, relationType.size() - 2);

                 ++it;
                 auto memberUriList = getValue<std::string_view>((*it).value());
                 memberUriList = memberUriList.substr(1, memberUriList.size() - 2);

                 ++it;
                 auto memberRolesList = getValue<std::string_view>((*it).value());
                 memberRolesList = memberRolesList.substr(1, memberRolesList.size() - 2);

                 ++it;
                 auto memberPosList = getValue<std::string_view>((*it).value());
                 memberPosList = memberPosList.substr(1, memberPosList.size() - 2);

                 const auto relationId = OsmObjectHelper::parseIdFromUri(relationUri);
                 const auto members = OsmObjectHelper::parseRelationMemberList(
                         memberUriList, memberRolesList, memberPosList);

                 // Write relation to file
                 const auto relationXml = util::XmlHelper::getRelationDummy(
                     relationId,relationType, members);
                 outputFile.write(relationXml.data(), relationXml.size());
                 outputFile << std::endl;
             });

    outputFile.close();
    return returnedRelationsCount;
}

// _________________________________________________________________________________________________
size_t olu::osm::OsmDataFetcherQLever::fetchAndWriteWaysToFile(const std::string &filePath,
                                                               const std::set<id_t> &wayIds) {
    std::ofstream outputFile;
    outputFile.open (filePath, std::ios::app);

    size_t returnedWayCount = 0;
    runQuery(_queryWriter.writeQueryForWaysMembers(wayIds), cnst::PREFIXES_FOR_WAY_MEMBERS,
             [&outputFile, &returnedWayCount, this](simdjson::ondemand::value results) {
                 returnedWayCount++;

                 auto it = results.begin();
                 const auto wayUri = getValue<std::string_view>((*it).value());

                 ++it;
                 auto memberUriList = getValue<std::string_view>((*it).value());
                 // Remove the surrounding brackets from the member URI list
                 memberUriList = memberUriList.substr(1, memberUriList.size() - 2);

                 ++it;
                 auto memberPosList = getValue<std::string_view>((*it).value());
                 // Remove the surrounding brackets from the member pos list
                 memberPosList = memberPosList.substr(1, memberPosList.size() - 2);

                 // Extract way infos from response
                 const auto wayId = OsmObjectHelper::parseIdFromUri(wayUri);
                 auto members = OsmObjectHelper::parseWayMemberList(memberUriList, memberPosList);

                 // Write way to file
                 const auto wayXml = util::XmlHelper::getWayDummy(wayId, members);
                 outputFile.write(wayXml.data(), wayXml.size());
                 outputFile << std::endl;
             });

    outputFile.close();
    return returnedWayCount;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t> olu::osm::OsmDataFetcherQLever::fetchWaysMembers(
    const std::set<id_t> &wayIds) {
    std::vector<id_t> nodeIds;
    runQuery(_queryWriter.writeQueryForReferencedNodes(wayIds), cnst::PREFIXES_FOR_WAY_MEMBERS,
             [&nodeIds](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto nodeUri = getValue<std::string_view>(result.value());
                     nodeIds.emplace_back(OsmObjectHelper::parseIdFromUri(nodeUri));
                 }
             });

    return nodeIds;
}

// _________________________________________________________________________________________________
std::pair<std::vector<olu::id_t>, std::vector<olu::id_t> >
olu::osm::OsmDataFetcherQLever::fetchRelationMembers(const std::set<id_t> &relIds) {
    std::vector<id_t> nodeIds;
    std::vector<id_t> wayIds;
    runQuery(_queryWriter.writeQueryForRelationMemberIds(relIds),
             cnst::PREFIXES_FOR_RELATION_MEMBERS,
             [&nodeIds, &wayIds](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto memberUri = getValue<std::string_view>(result.value());
                     id_t memberId = OsmObjectHelper::parseIdFromUri(memberUri);

                     // Skip the first char in member URI, because it is a special character ('<')
                     // when using Qlever responses
                     if (memberUri.substr(1).starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
                         nodeIds.emplace_back(memberId);
                     } else if (memberUri.substr(1).starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
                         wayIds.emplace_back(memberId);
                     }
                 }
             });

    return {nodeIds, wayIds};
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherQLever::fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) {
    std::vector<id_t> memberSubjects;

    runQuery(_queryWriter.writeQueryForWaysReferencingNodes(nodeIds),
             cnst::PREFIXES_FOR_WAYS_REFERENCING_NODE,
             [&memberSubjects, this](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto memberUri = getValue<std::string_view>(result.value());
                     memberSubjects.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
                 }
             });

    return memberSubjects;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherQLever::fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) {
    std::vector<id_t> relationIds;
    runQuery(_queryWriter.writeQueryForRelationsReferencingNodes(nodeIds),
             cnst::PREFIXES_FOR_RELATIONS_REFERENCING_NODE,
             [&relationIds, this](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto memberUri = getValue<std::string_view>(result.value());
                     relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
                 }
             });

    return relationIds;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherQLever::fetchRelationsReferencingWays(const std::set<id_t> &wayIds) {
    std::vector<id_t> relationIds;
    runQuery(_queryWriter.writeQueryForRelationsReferencingWays(wayIds),
             cnst::PREFIXES_FOR_RELATIONS_REFERENCING_WAY,
             [&relationIds, this](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto memberUri = getValue<std::string_view>(result.value());
                     relationIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
                 }
             });

    return relationIds;
}

// _________________________________________________________________________________________________
std::vector<olu::id_t>
olu::osm::OsmDataFetcherQLever::fetchRelationsReferencingRelations(
    const std::set<id_t> &relationIds) {
    std::vector<id_t> refRelIds;
    runQuery(_queryWriter.writeQueryForRelationsReferencingRelations(relationIds),
             cnst::PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS,
             [&refRelIds, this](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto memberUri = getValue<std::string_view>(result.value());
                     refRelIds.emplace_back(OsmObjectHelper::parseIdFromUri(memberUri));
                 }
             });

    return refRelIds;
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherQLever::fetchOsm2RdfVersion() {
    std::set<std::string> versions;

    runQuery(_queryWriter.writeQueryForOsm2RdfVersion(), cnst::PREFIXES_FOR_OSM2RDF_VERSION,
             [&versions](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     auto version = getValue<std::string>(result.value());
                     versions.insert(util::XmlHelper::parseRdfString<std::string>(version));
                 }
             });

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
std::map<std::string, std::string> olu::osm::OsmDataFetcherQLever::fetchOsm2RdfOptions() {
    std::map<std::string, std::string> options;

    runQuery(_queryWriter.writeQueryForOsm2RdfOptions(), cnst::PREFIXES_FOR_OSM2RDF_OPTIONS,
             [&options](simdjson::ondemand::value results) {
                 auto it = results.begin();
                 const auto optionIRI = getValue<std::string>((*it).value());

                 ++it;
                 const auto optionValue = getValue<std::string>((*it).value());

                 options.insert_or_assign(OsmObjectHelper::parseOsm2rdfOptionName(optionIRI),
                                          util::XmlHelper::parseRdfString<std::string>(optionValue));
             });

    return options;
}

// _________________________________________________________________________________________________
olu::osm::OsmDatabaseState olu::osm::OsmDataFetcherQLever::fetchUpdatesCompleteUntil() {
    std::set<OsmDatabaseState> updatesCompleteUntilResponses;

    runQuery(_queryWriter.writeQueryForUpdatesCompleteUntil(),
             cnst::PREFIXES_FOR_METADATA_TRIPLES,
             [&updatesCompleteUntilResponses](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     try {
                         auto seqNumResponse = getValue<std::string>(result.value());
                         auto databaseState = util::XmlHelper::parseRdfString<std::string>(seqNumResponse);
                         updatesCompleteUntilResponses.insert(from_string(databaseState));
                     } catch (std::exception &e) {
                         util::Logger::log(util::LogEvent::WARNING,
                                           "SPARQL endpoint returned invalid database state for "
                                           "'osm2rdfmeta:updatesCompleteUntil' predicate: "
                                           + std::string(e.what()));
                     }
                 }
             });

    if (const auto maxIt = std::max_element(updatesCompleteUntilResponses.begin(),
                                            updatesCompleteUntilResponses.end());
        maxIt != updatesCompleteUntilResponses.end()) {
        const OsmDatabaseState &maxState = *maxIt;
        return maxState;
        }

    throw OsmDataFetcherException("SPARQL endpoint did not return a valid database state.");
}

// _________________________________________________________________________________________________
std::string olu::osm::OsmDataFetcherQLever::fetchReplicationServer() {
    std::set<std::string> replicationServers;

    runQuery(_queryWriter.writeQueryForReplicationServer(),
             cnst::PREFIXES_FOR_METADATA_TRIPLES,
             [&replicationServers](simdjson::ondemand::value results) {
                 for (auto result: results) {
                     try {
                         auto replicationServer = getValue<std::string>(result.value());
                         replicationServer = util::XmlHelper::parseRdfString<std::string>(replicationServer);
                         replicationServers.insert(replicationServer);
                     } catch (std::exception &e) {
                         util::Logger::log(util::LogEvent::WARNING,
                                           "SPARQL endpoint returned invalid replication server uri: "
                                           + std::string(e.what()));
                     }
                 }
             });

    if (replicationServers.size() > 1) {
        util::Logger::log(util::LogEvent::WARNING,
                "SPARQL endpoint returned multiple replication server uris");
    }

    return replicationServers.empty() ? "" : *replicationServers.begin();
}

// _________________________________________________________________________________________________
template<typename T>
std::vector<T>
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

// _________________________________________________________________________________________________
void
olu::osm::OsmDataFetcherQLever::forResults(const simdjson::padded_string &response,
                                           const std::function<void(simdjson::ondemand::value)>
                                           func) {
    auto doc = _parser.iterate(response);
    if (doc.error()) {
        std::cerr << doc.error() << std::endl;
        throw OsmDataFetcherException("Error while parsing response document.");
    }

    for (auto result: getResults(doc.value())) {
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
template<typename T>
T
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
