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

#ifndef OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H
#define OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H

#include "sparql/SparqlWrapper.h"
#include "config/Config.h"
#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcher.h"
#include "config/Stats.h"
#include "gtest/gtest.h"

#include <set>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;
namespace olu::osm {

    /**
     * Processes a file in osm change format ('https://wiki.openstreetmap.org/wiki/OsmChange').
     *
     * An osm change file consists of changesets which can have three possible types: create, delete
     * and modify:
     *
     * - Osm elements within the `create` changeset are inserted to the database.
     * - Osm elements within the `delete` changeset are deleted from the database.
     * - Osm elements within the `modify` changeset are modified in the database.
     */
    class OsmChangeHandler {
    public:
        explicit OsmChangeHandler(config::Config &config,const std::string& pathToOsmChangeFile);

        /**
         * Process the content of the changeFile. We loop two times over the changeFile.
         * The first time we store the ids of all occuring elements in the corresponding array (
         * _createdNodes, _modifiedNodes, _deletedNodes, etc.). The second time we
         */
        void processChangeFile();
        void storeIdsOfElementsInChangeFile();
        void processElementsInChangeFile();

        void getIdsForGeometryUpdate();
        void getReferencedRelations();
        void getReferencedWays();
        void getReferencedNodes();

        void run();

        /**
         * Counts the osm elements (`node`, `way` or `relation`) in the osm change file that are in
         * contained in the changesets.
         *
         * @param osmChangeElement the osm change element
         * @return The number of elements to be processed
         */
        static size_t countElements(const boost::property_tree::ptree &osmChangeElement);

        /**
         * Creates a SPARQL query from the given ttl data to add the contained triples into the
         * database and sends it to the SPARQL endpoint.
         *
         * @param convertedData The ttl data containing the triples that should be inserted to the datatbase
         * as well as the needed prefixes
         */
        void createAndRunInsertQuery();

    private:
        config::Config _config;
        sparql::SparqlWrapper _sparql;
        Osm2ttl _osm2ttl;
        OsmDataFetcher _odf;
        Stats _stats;

        boost::property_tree::ptree _osmChangeElement;

        // Nodes that are in a delete-changeset in the change file.
        std::set<long long> _deletedNodes;
        // Nodes that are in a create-changeset in the change file.
        std::set<long long> _createdNodes;
        // Nodes that are in a modify-changeset in the change file.
        std::set<long long> _modifiedNodes;
        // Nodes that are referenced by a way or relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<long long> _referencedNodes;

        // Ways that are in a delete-changeset in the change file.
        std::set<long long> _deletedWays;
        // Ways that are in a create-changeset in the change file.
        std::set<long long> _createdWays;
        // Ways that are in a modify-changeset in the change file.
        std::set<long long> _modifiedWays;
        // Ways that reference a node which was modified in the changeset.
        std::set<long long> _waysToUpdateGeometry;
        // Ways that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<long long> _referencedWays;

        // Relations that are in a delete-changeset in the change file.
        std::set<long long> _deletedRelations;
        // Relations that are in a create-changeset in the change file.
        std::set<long long> _createdRelations;
        // Relations that are in a modify-changeset in the change file.
        std::set<long long> _modifiedRelations;
        // Relations that reference a node, way or relation which was modified in the changeset.
        std::set<long long> _relationsToUpdateGeometry;
        // Relations that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<long long> _referencedRelations;

        /**
         * @Returns TRUE if the node with the given ID is contained in a `create` or `modify`
         * changeset in the changeFile.
         *
         * @warning All nodes inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        bool nodeInChangeFile(const long long &nodeId) {
            return _modifiedNodes.contains(nodeId) || _createdNodes.contains(nodeId);
        }

        /**
         * @Returns TRUE if the way with the given ID is contained in a `create` or `modify`
         * changeset in the changeFile.
         *
         * @warning All ways inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        bool wayInChangeFile(const long long &wayId) {
            return _modifiedWays.contains(wayId) || _createdWays.contains(wayId);
        }

        /**
         * @Returns TRUE if the relation with the given ID is contained in a `create` or `modify`
         * changeset in the changeFile.
         *
         * @warning All relations inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is after calling
         * `storeIdsOfElementsInChangeFile()`
         */
        bool relationInChangeFile(const long long &relationId) {
            return _modifiedRelations.contains(relationId) || _createdRelations.contains(relationId);
        }

        static void createOrClearTmpFiles() ;
        static void addToTmpFile(const boost::property_tree::ptree& element, const std::string& elementTag) ;
        static void addToTmpFile(const std::string& element, const std::string& elementTag) ;

        void storeIdsOfReferencedNodes(const boost::property_tree::ptree& wayElement);
        void storeIdsOfReferencedElements(const boost::property_tree::ptree& relElement);

        /**
         * Creates an vector containing dummy nodes for the given node ids. The dummy nodes contain
         * the node id and the location which is used for the nodes that are referenced in ways.
         *
         * @param nodeIds The node ids to create dummy nodes for
         * @return A vector containing a dummy node for each given node id
         */
        void createDummyNodes();
        void createDummyWays();
        void createDummyRelations();
        static void sortFile(const std::string& elementTag);
        void deleteElementsFromDatabase();
        void insertElementsToDatabase();
        void filterRelevantTriples();
    };

    /**
     * Exception that can appear inside the `OsmChangeHandler` class.
     */
    class OsmChangeHandlerException : public std::exception {
    private:
        std::string message;
    public:
        explicit OsmChangeHandlerException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H
