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
#include "config/Constants.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"
#include "util/XmlReader.h"
#include "sparql/QueryWriter.h"

#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <fstream>

#include "osm2rdf/osm/RelationMember.h"

namespace constants = olu::config::constants;

namespace olu::osm {
    class Relation;

    // _____________________________________________________________________________________________
    OsmDatabaseState OsmDataFetcher::fetchDatabaseState(int sequenceNumber) const {
        // Build url for state file
        std::string seqNumberFormatted =
                util::URLHelper::formatSequenceNumberForUrl(sequenceNumber);
        std::string stateFileName =
                seqNumberFormatted + ".state.txt";

        std::vector<std::string> pathSegments { };
        pathSegments.emplace_back(_config.osmChangeFileDirectoryUri);
        pathSegments.emplace_back(stateFileName);
        std::string url = util::URLHelper::buildUrl(pathSegments);

        //  state file from osm server
        auto request = util::HttpRequest(util::GET, url);

        std::string response;
        response = request.perform();

        return extractStateFromStateFile(response);
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState OsmDataFetcher::fetchLatestDatabaseState() const {
        // Build url for state file
        std::vector<std::string> pathSegments { };
        pathSegments.emplace_back(_config.osmChangeFileDirectoryUri);
        pathSegments.emplace_back("state.txt");
        std::string url = util::URLHelper::buildUrl(pathSegments);

        // Get state file from osm server
        auto request = util::HttpRequest(util::GET, url);
        std::string response = request.perform();
        return extractStateFromStateFile(response);
    }

    // _____________________________________________________________________________________________
    std::string OsmDataFetcher::fetchChangeFile(int &sequenceNumber) {
        // Build url for change file
        std::string sequenceNumberFormatted = util::URLHelper::formatSequenceNumberForUrl(sequenceNumber);
        std::string diffFilename = sequenceNumberFormatted + constants::OSM_CHANGE_FILE_EXTENSION + constants::GZIP_EXTENSION;
        std::vector<std::string> pathSegments;
        pathSegments.emplace_back(_config.osmChangeFileDirectoryUri);
        pathSegments.emplace_back(diffFilename);
        std::string url = util::URLHelper::buildUrl(pathSegments);

        // Get change file from server and write to cache file.
        std::string filePath = constants::DIFF_CACHE_FILE + std::to_string(sequenceNumber) + constants::OSM_CHANGE_FILE_EXTENSION + constants::GZIP_EXTENSION;
        auto request = util::HttpRequest(util::GET, url);

        auto response = request.perform();
        std::ofstream outputFile;
        outputFile.open (filePath);
        outputFile << response;
        outputFile.close();

        return filePath;
    }

    std::vector<Node>
    OsmDataFetcher::fetchNodes(const std::set<long long int> &nodeIds) {
        auto query = sparql::QueryWriter::writeQueryForNodeLocations(nodeIds);
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_NODE_LOCATION);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<Node> nodes;
        for (const auto &result : responseAsTree.get_child("sparql.results")) {
            u_id id;
            std::string locationAsWkt;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name", binding.second);
                if (name == "nodeGeo") {
                    auto uri = binding.second.get<std::string>("uri");

                    try {
                        id = std::stoll(uri.substr(constants::OSM_GEOM_NODE_URI.length()));
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << std::endl;
                        std::string msg = "Exception while trying to get id from uri: " + uri;
                        throw OsmDataFetcherException(msg.c_str());
                    }
                }

                if (name == "location") {
                    locationAsWkt = binding.second.get<std::string>("literal");
                }
            }

            nodes.emplace_back( id, locationAsWkt );
        }

        if (nodes.size() > nodeIds.size()) {
            std::cout
                << "The SPARQL endpoint returned " << std::to_string(nodes.size())
                << " locations, for " << std::to_string(nodeIds.size())
                << " nodes." << std::endl;
            throw OsmDataFetcherException("Exception while trying to fetch nodes locations");
        }

        return nodes;
    }

    // _____________________________________________________________________________________________
    std::string OsmDataFetcher::fetchLatestTimestampOfAnyNode() {
        auto query = olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp();
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_LATEST_NODE_TIMESTAMP);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::string timestamp;
        try {
            timestamp = responseAsTree.get<std::string>(
                    constants::PATH_TO_SPARQL_RESULT);
        } catch (boost::property_tree::ptree_bad_path &e) {
            std::cerr
            << "Could not fetch latest timestamp of any node from sparql endpoint"
            << std::endl;
            throw OsmDataFetcherException(
                    "Could not fetch latest timestamp of any node from sparql endpoint");
        }

        return timestamp;
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState
    OsmDataFetcher::fetchDatabaseStateForTimestamp(const std::string& timeStamp) const {
        OsmDatabaseState state = fetchLatestDatabaseState();
        while (true) {
            if (state.timeStamp > timeStamp) {
                auto seq = state.sequenceNumber;
                seq--;
                state = fetchDatabaseState(seq);
            } else {
                return state;
            }
        }
    }

    // _____________________________________________________________________________________________
    OsmDatabaseState OsmDataFetcher::extractStateFromStateFile(const std::string& stateFile) {
        OsmDatabaseState ods;
        // Extract sequence number from state file
        boost::regex regexSeqNumber("sequenceNumber=(\\d+)");
        boost::smatch matchSeqNumber;
        if (boost::regex_search(stateFile, matchSeqNumber, regexSeqNumber)) {
            std::string number = matchSeqNumber[1];
            ods.sequenceNumber = std::stoi(number);
        } else {
            throw OsmDataFetcherException(
                    "Sequence number of latest database state could not be fetched");
        }

        // Extract timestamp from state file
        boost::regex regexTimestamp(
                R"(timestamp=([0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}\\:[0-9]{2}\\:[0-9]{2}Z))");
        boost::smatch matchTimestamp;
        if (boost::regex_search(stateFile, matchTimestamp, regexTimestamp)) {
            std::string timestamp = matchTimestamp[1];
            ods.timeStamp = timestamp;
        } else {
            throw OsmDataFetcherException(
                    "Timestamp of latest database state could not be fetched");
        }

        return ods;
    }

    // _________________________________________________________________________________________________
    std::vector<Relation>
    OsmDataFetcher::fetchRelations(const std::set<long long int> &relationIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForRelationMembers(relationIds);
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_RELATION_MEMBERS);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        Relation currentRelation(0);
        std::vector<Relation> relations;
        for (const auto &result : responseAsTree.get_child("sparql.results")) {
            std::string memberUri;
            std::string role;
            id relationId;
            std::string relationType;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name",binding.second);
                if (name == "rel") {
                    auto relUri = binding.second.get<std::string>("uri");
                    relationId = std::stoll(relUri.substr(constants::OSM_REL_URI.length()));
                }

                if (name == "key") {
                    relationType = binding.second.get<std::string>("literal");
                }

                if (name == "id") {
                    memberUri = binding.second.get<std::string>("uri");
                }

                if (name == "role") {
                    role = binding.second.get<std::string>("literal");
                }
            }

            if (currentRelation.getId() != relationId) {
                relations.push_back(currentRelation);
                currentRelation = Relation(relationId);
                currentRelation.setType(relationType);
            }

            if (memberUri.starts_with(constants::OSM_NODE_URI)) {
                u_id nodeId = std::stoll( memberUri.substr(constants::OSM_WAY_URI.length()));
                currentRelation.addNodeAsMember(nodeId, role);
            } else if (memberUri.starts_with(constants::OSM_WAY_URI)) {
                u_id wayId = std::stoll( memberUri.substr(constants::OSM_WAY_URI.length()));
                currentRelation.addNodeAsMember(wayId, role);
            } else if (memberUri.starts_with(constants::OSM_REL_URI)) {
                u_id relId = std::stoll( memberUri.substr(constants::OSM_WAY_URI.length()));
                currentRelation.addNodeAsMember(relId, role);
            }
        }

        // Delete first placeholder way from list
        if (!relations.empty()) {
            relations.erase(relations.begin());
        }
        return relations;
    }

    std::vector<Way> OsmDataFetcher::fetchWays(const std::set<long long> &wayIds) {
        auto query = sparql::QueryWriter::writeQueryForWaysMembers(wayIds);
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_WAY_MEMBERS);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::map<u_id, std::vector<u_id>> wayMap;
        for (const auto &result : responseAsTree.get_child("sparql.results")) {
            u_id wayId;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name", binding.second);
                if (name == "way") {
                    auto uri = binding.second.get<std::string>("uri");
                    wayId = std::stoll( uri.substr(constants::OSM_WAY_URI.length()) );

                    if (!wayMap.contains(wayId)) {
                        wayMap[wayId] = std::vector<u_id>();
                    }
                }

                if (name == "node") {
                    auto uri = binding.second.get<std::string>("uri");
                    u_id nodeId = std::stoll( uri.substr(constants::OSM_NODE_URI.length()) );
                    wayMap[wayId].push_back(nodeId);
                }
            }
        }

        std::vector<Way> ways;
        for (const auto &[wayId, nodeIds] : wayMap) {
            Way way(wayId);
            for (const auto &nodeId : nodeIds) {
                way.addMember(nodeId);
            }
            ways.push_back(way);
        }

        return ways;
    }

    std::vector<long long> OsmDataFetcher::fetchWaysMembers(const std::set<long long> &wayIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForReferencedNodes(wayIds);
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_WAY_MEMBERS);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> nodeIds;
        for (const auto &result : responseAsTree.get_child("sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            // The endpoint will return the uri of the way, so we have to extract the id from it
            std::string nodeIdAsString = memberSubject.substr(constants::OSM_NODE_URI.length());
            long long nodeId = -1;
            try {
                nodeId = std::stoll(nodeIdAsString);

                if (nodeId < 0) {
                    throw;
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Could extract way id from uri: " + memberSubject;
                throw OsmDataFetcherException(msg.c_str());
            }

            nodeIds.emplace_back(nodeId);
        }

        return nodeIds;
    }

    std::pair<std::vector<long long int>, std::vector<long long int>>
    OsmDataFetcher::fetchRelationMembers(const std::set<long long> &relIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForRelationMembers(relIds);
        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_RELATION_MEMBERS);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> nodeIds;
        std::vector<long long> wayIds;
        for (const auto &result : responseAsTree.get_child("sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            long long id = -1;
            if (memberSubject.starts_with(constants::OSM_NODE_URI)) {
                std::string nodeIdAsString = memberSubject.substr(constants::OSM_NODE_URI.length());
                try {
                    id = std::stoll(nodeIdAsString);
                    nodeIds.emplace_back(id);

                    if (id < 0) {
                        throw;
                    }
                } catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    std::string msg = "Could extract way id from uri: " + memberSubject;
                    throw OsmDataFetcherException(msg.c_str());
                }
            } else if (memberSubject.starts_with(constants::OSM_WAY_URI)) {
                std::string wayIdAsString = memberSubject.substr(constants::OSM_WAY_URI.length());
                try {
                    id = std::stoll(wayIdAsString);
                    wayIds.emplace_back(id);

                    if (id < 0) {
                        throw;
                    }
                } catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    std::string msg = "Could extract way id from uri: " + memberSubject;
                    throw OsmDataFetcherException(msg.c_str());
                }
            }
        }

        return { nodeIds, wayIds };
    }

    // _________________________________________________________________________________________________
    std::vector<long long> OsmDataFetcher::fetchWaysReferencingNodes(
            const std::set<long long int> &nodeIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForWaysReferencingNodes(nodeIds);

        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_WAYS_REFERENCING_NODE);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> memberSubjects;
        for (const auto &result : responseAsTree.get_child(
                "sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            // The endpoint will return the uri of the way, so we have to extract the id from it
            std::string wayIdAsString = memberSubject.substr(constants::OSM_WAY_URI.length());
            long long wayId = -1;
            try {
                wayId = std::stoll(wayIdAsString);

                if (wayId < 0) {
                    throw;
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Could extract way id from uri: " + memberSubject;
                throw OsmDataFetcherException(msg.c_str());
            }

            memberSubjects.emplace_back(wayId);
        }

        return memberSubjects;
    }

    // _________________________________________________________________________________________________
    std::vector<long long> OsmDataFetcher::fetchRelationsReferencingNodes(
            const std::set<long long int> &nodeIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingNodes(nodeIds);

        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_RELATIONS_REFERENCING_NODE);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> memberSubjects;
        for (const auto &result : responseAsTree.get_child(
                "sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            // The endpoint will return the uri of the way, so we have to extract the id from it
            std::string wayIdAsString = memberSubject.substr(constants::OSM_REL_URI.length());
            long long wayId = -1;
            try {
                wayId = std::stoll(wayIdAsString);

                if (wayId < 0) {
                    throw;
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Could extract way id from uri: " + memberSubject;
                throw OsmDataFetcherException(msg.c_str());
            }

            memberSubjects.emplace_back(wayId);
        }

        return memberSubjects;
    }

    // _________________________________________________________________________________________________
    std::vector<long long> OsmDataFetcher::fetchRelationsReferencingWays(
            const std::set<long long int> &wayIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingWays(wayIds);

        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_RELATIONS_REFERENCING_WAY);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> memberSubjects;
        for (const auto &result : responseAsTree.get_child(
                "sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            // The endpoint will return the uri of the way, so we have to extract the id from it
            std::string wayIdAsString = memberSubject.substr(constants::OSM_REL_URI.length());
            long long wayId = -1;
            try {
                wayId = std::stoll(wayIdAsString);

                if (wayId < 0) {
                    throw;
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Could extract way id from uri: " + memberSubject;
                throw OsmDataFetcherException(msg.c_str());
            }

            memberSubjects.emplace_back(wayId);
        }

        return memberSubjects;
    }

    // _________________________________________________________________________________________________
    std::vector<long long> OsmDataFetcher::fetchRelationsReferencingRelations(
            const std::set<long long int> &relationIds) {
        auto query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingRelations(relationIds);

        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(constants::PREFIXES_FOR_RELATIONS_REFERENCING_RELATIONS);
        auto response = _sparqlWrapper.runQuery();

        boost::property_tree::ptree responseAsTree;
        olu::util::XmlReader::populatePTreeFromString(response, responseAsTree);

        std::vector<long long> memberSubjects;
        for (const auto &result : responseAsTree.get_child(
                "sparql.results")) {
            auto memberSubject = result.second.get<std::string>("binding.uri");

            // The endpoint will return the uri of the way, so we have to extract the id from it
            std::string wayIdAsString = memberSubject.substr(constants::OSM_REL_URI.length());
            long long relId = -1;
            try {
                relId = std::stoll(wayIdAsString);

                if (relId < 0) {
                    throw;
                }
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                std::string msg = "Could extract relation id from uri: " + memberSubject;
                throw OsmDataFetcherException(msg.c_str());
            }

            memberSubjects.emplace_back(relId);
        }

        return memberSubjects;
    }

    // _________________________________________________________________________________________________
    long long OsmDataFetcher::getIdFor(const boost::property_tree::ptree &element) {
        std::string identifier;
        try {
            identifier = olu::util::XmlReader::readAttribute(
                    olu::config::constants::ID_ATTRIBUTE,
                    element);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::string msg = "Could not identifier of element: " + util::XmlReader::readTree(element);
            throw OsmDataFetcherException(msg.c_str());
        }

        try {
            return std::stoll(identifier);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            std::string msg = "Could not cast identifier: " + identifier + " to long long";
            throw OsmDataFetcherException(msg.c_str());
        }
    }
} // namespace olu



