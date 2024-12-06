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
#include "util/OsmObjectHelper.h"
#include "util/XmlReader.h"
#include "sparql/QueryWriter.h"

#include <vector>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <fstream>

namespace cnst = olu::config::constants;
namespace olu::osm {
    // _____________________________________________________________________________________________
    boost::property_tree::ptree OsmDataFetcher::runQuery(
        const std::string &query,
        const std::vector<std::string> &prefixes) {

        _sparqlWrapper.setQuery(query);
        _sparqlWrapper.setPrefixes(prefixes);
        return _sparqlWrapper.runQuery();
    }

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
        const std::string url = util::URLHelper::buildUrl(pathSegments);

        // Get state file from osm server
        auto request = util::HttpRequest(util::GET, url);
        const std::string response = request.perform();
        return extractStateFromStateFile(response);
    }

    // _____________________________________________________________________________________________
    std::string OsmDataFetcher::fetchChangeFile(int &sequenceNumber) {
        // Build url for change file
        std::string sequenceNumberFormatted = util::URLHelper::formatSequenceNumberForUrl(
            sequenceNumber);
        std::string diffFilename = sequenceNumberFormatted + cnst::OSM_CHANGE_FILE_EXTENSION +
                                   cnst::GZIP_EXTENSION;
        std::vector<std::string> pathSegments;
        pathSegments.emplace_back(_config.osmChangeFileDirectoryUri);
        pathSegments.emplace_back(diffFilename);
        std::string url = util::URLHelper::buildUrl(pathSegments);

        // Get change file from server and write to cache file.
        std::string filePath = cnst::DIFF_CACHE_FILE + std::to_string(sequenceNumber) +
                               cnst::OSM_CHANGE_FILE_EXTENSION + cnst::GZIP_EXTENSION;
        auto request = util::HttpRequest(util::GET, url);

        auto response = request.perform();
        std::ofstream outputFile;
        outputFile.open(filePath);
        outputFile << response;
        outputFile.close();

        return filePath;
    }

    // _____________________________________________________________________________________________
    std::vector<Node>
    OsmDataFetcher::fetchNodes(const std::set<id_t> &nodeIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForNodeLocations(nodeIds),
            cnst::PREFIXES_FOR_NODE_LOCATION);

        std::vector<Node> nodes;
        for (const auto &result : response.get_child("sparql.results")) {
            id_t id;
            std::string locationAsWkt;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name", binding.second);
                if (name == "nodeGeo") {
                    auto uri = binding.second.get<std::string>("uri");
                    id = OsmObjectHelper::getIdFromUri(uri);
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
        const auto response = runQuery(
            _queryWriter.writeQueryForLatestNodeTimestamp(),
            cnst::PREFIXES_FOR_LATEST_NODE_TIMESTAMP);

        std::string timestamp;
        try {
            timestamp = response.get<std::string>("sparql.results.result.binding.literal");
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
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
        if (boost::smatch matchSeqNumber; regex_search(stateFile, matchSeqNumber, regexSeqNumber)) {
            std::string number = matchSeqNumber[1];
            ods.sequenceNumber = std::stoi(number);
        } else {
            throw OsmDataFetcherException(
                    "Sequence number of latest database state could not be fetched");
        }

        // Extract timestamp from state file
        boost::regex regexTimestamp(
                R"(timestamp=([0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}\\:[0-9]{2}\\:[0-9]{2}Z))");
        if (boost::smatch matchTimestamp; regex_search(stateFile, matchTimestamp, regexTimestamp)) {
            std::string timestamp = matchTimestamp[1];
            ods.timeStamp = timestamp;
        } else {
            throw OsmDataFetcherException(
                    "Timestamp of latest database state could not be fetched");
        }

        return ods;
    }

    // _____________________________________________________________________________________________
    std::vector<Relation>
    OsmDataFetcher::fetchRelations(const std::set<id_t> &relationIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelations(relationIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<Relation> relations;
        for (const auto &result : response.get_child("sparql.results")) {
            id_t relationId;
            std::string type;
            std::string memberUris;
            std::string memberRoles;
            std::string memberPositions;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name",binding.second);
                if (name == "rel") {
                    auto relUri = binding.second.get<std::string>("uri");
                    relationId = OsmObjectHelper::getIdFromUri(relUri);
                }

                if (name == "type") {
                    type = binding.second.get<std::string>("literal");
                }

                if (name == "memberUris") {
                    memberUris = binding.second.get<std::string>("literal");
                }

                if (name == "memberRoles") {
                    memberRoles = binding.second.get<std::string>("literal");
                }

                if (name == "memberPositions") {
                    memberPositions = binding.second.get<std::string>("literal");
                }
            }

            Relation relation(relationId);
            relation.setType(type);

            std::stringstream uriStream(memberUris);
            std::stringstream rolesStream(memberRoles);
            std::stringstream posStream(memberPositions);

            std::string uri;
            std::string role;
            std::string position;
            std::map<int, RelationMember> members;
            while (std::getline(uriStream, uri, ';')) {
                if (uri.empty()) { continue; }

                std::getline(rolesStream, role, ';');
                std::getline(posStream, position, ';');
                id_t id = OsmObjectHelper::getIdFromUri(uri);

                std::string osmTag;
                if (uri.starts_with(cnst::OSM_NODE_URI)) {
                    osmTag = cnst::NODE_TAG;
                } else if (uri.starts_with(cnst::OSM_WAY_URI)) {
                    osmTag = cnst::WAY_TAG;
                } else if (uri.starts_with(cnst::OSM_REL_URI)) {
                    osmTag = cnst::RELATION_TAG;
                }
                members.emplace(std::stoi(position), RelationMember(id, osmTag, role));
            }

            for (auto [pos, member] : members) {
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
        for (const auto &result : response.get_child("sparql.results")) {
            id_t wayId;
            std::string nodeUris;
            std::string nodePositions;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name",binding.second);
                if (name == "way") {
                    auto wayUri = binding.second.get<std::string>("uri");
                    wayId = OsmObjectHelper::getIdFromUri(wayUri);
                }

                if (name == "nodeUris") {
                    nodeUris = binding.second.get<std::string>("literal");
                }

                if (name == "nodePositions") {
                    nodePositions = binding.second.get<std::string>("literal");
                }
            }

            Way way(wayId);

            std::stringstream uriStream(nodeUris);
            std::stringstream posStream(nodePositions);

            std::string uri;
            std::string position;
            std::map<int, id_t> nodes;
            while (std::getline(uriStream, uri, ';')) {
                if (uri.empty()) { continue; }

                std::getline(posStream, position, ';');
                id_t id = OsmObjectHelper::getIdFromUri(uri);
                nodes.emplace(std::stoi(position), id);
            }

            for (auto [pos, nodeId] : nodes) {
                way.addMember(nodeId);
            }
            ways.emplace_back(way);
        }

        return ways;
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcher::fetchWayInfos(Way &way) {
        std::string subject = "osmway:" + std::to_string(way.getId());
        auto response = runQuery(
            _queryWriter.writeQueryForTagsAndTimestamp(subject),
            cnst::PREFIXES_FOR_WAY_TAGS);

        for (const auto &result : response.get_child("sparql.results")) {
            std::string key; std::string value;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name", binding.second);
                if (name == "time") {
                    way.setTimestamp(binding.second.get<std::string>("literal"));
                    continue;
                }

                if (name == "key") {
                    auto uri = binding.second.get<std::string>("uri");
                    key = uri.substr(cnst::OSM_TAG_KEY.length());
                }

                if (name == "value") {
                    value = binding.second.get<std::string>("literal");
                }
            }

            if (!key.empty()) {
                way.addTag(key, value);
            }
        }
    }

    // _____________________________________________________________________________________________
    void OsmDataFetcher::fetchRelationInfos(Relation &relation) {
        std::string subject = "osmrel:" + std::to_string(relation.getId());
        auto response = runQuery(
            _queryWriter.writeQueryForTagsAndTimestamp(subject),
            cnst::PREFIXES_FOR_RELATION_TAGS);

        for (const auto &result : response.get_child("sparql.results")) {
            std::string key; std::string value;
            for (const auto &binding : result.second.get_child("")) {
                auto name = util::XmlReader::readAttribute("<xmlattr>.name", binding.second);
                if (name == "time") {
                    relation.setTimestamp(binding.second.get<std::string>("literal"));
                    continue;
                }

                if (name == "key") {
                    auto uri = binding.second.get<std::string>("uri");
                    key = uri.substr(cnst::OSM_TAG_KEY.length());
                }

                if (name == "value") {
                    value = binding.second.get<std::string>("literal");
                }
            }

            // Type of relation is already fetched in an earlier step
            if (!key.empty() && key != "type") {
                relation.addTag(key, value);
            }
        }
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcher::fetchWaysMembers(const std::set<id_t> &wayIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForReferencedNodes(wayIds),
            cnst::PREFIXES_FOR_WAY_MEMBERS);

        std::vector<id_t> nodeIds;
        for (const auto &result : response.get_child("sparql.results")) {
            auto memberUri = result.second.get<std::string>("binding.uri");
            nodeIds.emplace_back(OsmObjectHelper::getIdFromUri(memberUri));
        }

        return nodeIds;
    }

    // _____________________________________________________________________________________________
    std::pair<std::vector<id_t>, std::vector<id_t>>
    OsmDataFetcher::fetchRelationMembers(const std::set<id_t> &relIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationMembers(relIds),
            cnst::PREFIXES_FOR_RELATION_MEMBERS);

        std::vector<id_t> nodeIds;
        std::vector<id_t> wayIds;
        for (const auto &result : response.get_child("sparql.results")) {
            auto memberUri = result.second.get<std::string>("binding.uri");

            id_t id = OsmObjectHelper::getIdFromUri(memberUri);
            if (memberUri.starts_with(cnst::OSM_NODE_URI)) {
                nodeIds.emplace_back(id);
            } else if (memberUri.starts_with(cnst::OSM_WAY_URI)) {
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
        for (const auto &result : response.get_child("sparql.results")) {
            auto memberUri = result.second.get<std::string>("binding.uri");
            memberSubjects.emplace_back(OsmObjectHelper::getIdFromUri(memberUri));
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
        for (const auto &result : response.get_child(
                "sparql.results")) {
            auto memberUri = result.second.get<std::string>("binding.uri");
            relationIds.emplace_back(OsmObjectHelper::getIdFromUri(memberUri));
        }

        return relationIds;
    }

    // _____________________________________________________________________________________________
    std::vector<id_t> OsmDataFetcher::fetchRelationsReferencingWays(const std::set<id_t> &wayIds) {
        auto response = runQuery(
            _queryWriter.writeQueryForRelationsReferencingWays(wayIds),
            cnst::PREFIXES_FOR_RELATIONS_REFERENCING_WAY);

        std::vector<id_t> relationIds;
        for (const auto &result : response.get_child("sparql.results")) {
            auto uri = result.second.get<std::string>("binding.uri");
            relationIds.emplace_back(OsmObjectHelper::getIdFromUri(uri));
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
        for (const auto &result : response.get_child("sparql.results")) {
            auto uri = result.second.get<std::string>("binding.uri");
            refRelIds.emplace_back(OsmObjectHelper::getIdFromUri(uri));
        }

        return refRelIds;
    }

}
