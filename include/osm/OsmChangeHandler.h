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

#include "osm/Osm2ttl.h"
#include "osm/OsmDataFetcher.h"
#include "sparql/SparqlWrapper.h"
#include "config/Config.h"
#include "osm2rdf/util/ProgressBar.h"
#include <set>
#include <boost/property_tree/ptree.hpp>


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
        explicit OsmChangeHandler(const config::Config &config);
        void run();
    private:
        config::Config _config;
        sparql::SparqlWrapper _sparql;
        sparql::QueryWriter _queryWriter;
        OsmDataFetcher _odf;

        // The xml element of the change file is stored here while processing the change file
        boost::property_tree::ptree _osmChangeElement;

        // Nodes that are in a delete-changeset in the change file.
        std::set<id_t> _deletedNodes;
        // Nodes that are in a create-changeset in the change file.
        std::set<id_t> _createdNodes;
        // Nodes that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedNodes;
        // Nodes that where modified in the changeset and have a location that has changed.
        std::set<id_t> _modifiedNodesWithChangedLocation;
        // Nodes that are referenced by a way or relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedNodes;

        // Ways that are in a delete-changeset in the change file.
        std::set<id_t> _deletedWays;
        // Ways that are in a create-changeset in the change file.
        std::set<id_t> _createdWays;
        // Ways that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedWays;
        // Ways that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedWaysWithChangedMembers;
        // Ways that reference a node which was modified in the changeset.
        std::set<id_t> _waysToUpdateGeometry;
        // Ways that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedWays;

        // Relations that are in a delete-changeset in the change file.
        std::set<id_t> _deletedRelations;
        // Relations that are in a create-changeset in the change file.
        std::set<id_t> _createdRelations;
        // Relations that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedRelations;
        // Relations that are of type multipolygon that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedAreas;
        // Relations that reference a node, way or relation which was modified in the changeset.
        std::set<id_t> _relationsToUpdateGeometry;
        // Relations that are referenced by a relation that are NOT present in the change file,
        // meaning they have to be fetched from the database
        std::set<id_t> _referencedRelations;

        /**
        * @Returns TRUE if the node with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All nodes inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        [[nodiscard]] bool nodeInChangeFile(const id_t &nodeId) const {
            return _modifiedNodes.contains(nodeId) ||
                   _modifiedNodesWithChangedLocation.contains(nodeId) ||
                   _createdNodes.contains(nodeId) ||
                   _deletedNodes.contains(nodeId);
        }

        /**
         * @Returns TRUE if the way with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All ways inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        [[nodiscard]] bool wayInChangeFile(const id_t &wayId) const {
            return _modifiedWays.contains(wayId) ||
                   _modifiedWaysWithChangedMembers.contains(wayId) ||
                   _createdWays.contains(wayId) ||
                   _deletedWays.contains(wayId);
        }

        /**
         * @Returns TRUE if the relation with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All relations inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is after calling
         * `storeIdsOfElementsInChangeFile()`
         */
        [[nodiscard]] bool relationInChangeFile(const id_t &relationId) const {
            return _modifiedRelations.contains(relationId) ||
                   _createdRelations.contains(relationId) ||
                   _deletedRelations.contains(relationId);
        }

        /**
         * Loops over the change file and stores the ids of all occurring elements in the
         * corresponding set (_createdNodes, _modifiedNodes, _deletedNodes, etc.).
         */
        void storeIdsOfElementsInChangeFile();

        /**
         * Checks if the location of the given nodes from the change file has changed. If so, the
         * node is added to the _modifiedNodesWithChangedLocation set, otherwise to the
         * _modifiedNodes set
         *
         * @param nodeIds The ids of the nodes to check
         * @param nodeLocs The locations of the nodes to check
         */
        void checkNodesForLocationChange(std::set<id_t> &nodeIds,
                                         const std::vector<osmium::Location> & nodeLocs);

        /**
         * Checks if the members of the given ways from the change file have changed. If so, the
         * way is added to the _modifiedWaysWithChangedMembers set, otherwise to the
         * _modifiedWays set
         *
         * @param waysWithMembers Pairs of wayIds with a vector containing the node references
         */
        void checkWaysForMemberChange(const std::vector<std::pair<id_t, member_ids_t>>& waysWithMembers);

        /**
         * Stores the ids of the nodes and ways that are referenced in the given relation or way in
         * the _referencedNodes or _referencedWays set
         */
        void storeIdsOfReferencedElements(const boost::property_tree::ptree& relElement);

        /**
         * Loops over the change file and stores the relevant ones in a temporary file, and the
         * referenced elements in the corresponding set
         */
        void processElementsInChangeFile();

        /**
         * Fetches the ids of ways and relations of which the geometry needs to be updated and
         * stores them in the corresponding set
         */
        void getIdsOfRelationsToUpdateGeo();
        void getIdsOfWaysToUpdateGeo();

        /**
         * Fetches the ids of relations that are referenced in relations which geometry will be
         * changed in this update process and stores them in the corresponding set
         *
         * @Warning This is currently skipped because osm2rdf does not calculate the geometries for
         * relations that reference other relations
         */
        void getReferencedRelations();

        /**
         * Fetches the ids of all ways that are referenced in relations which geometry will be
         * changed in this update process and stores them in the corresponding set
         */
        void getReferencesForRelations();

        /**
         * Fetches the ids of all nodes that are referenced in either ways or relations which
         * geometries will be needed and stores them in the corresponding set
         */
        void getReferencesForWays();

        static void createTmpFiles() ;
        static void initTmpFile(const std::string& filepath) ;
        static void finalizeTmpFile(const std::string& filepath) ;

        /**
         * Writes the given osm element to its corresponding temporary file
         */
        static void addToTmpFile(const std::string& element, const std::string& elementTag) ;

        /**
         * Creates dummy elements for nodes, ways and relations, while showing a progress bar on
         * std::cout
         */
        void createDummyElements();

        /**
         * Creates dummy nodes for the referenced nodes that are not in the change file. The dummy
         * nodes contain the node id and the location which is used for the nodes that are
         * referenced in ways and writes them to a temporary file
         */
        void createDummyNodes(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Creates dummy ways for the referenced ways that are not in the change file and writes
         * them to a temporary file The dummy ways only contain the referenced nodes
         */
        void createDummyWays(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Creates dummy relations for the referenced relations that are not in the change file and
         * writes them to a temporary file The dummy relation only contain the members of that
         * relation
         */
        void createDummyRelations(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send a SPARQL update query to the endpoint
         */
        void runUpdateQuery(const std::string& query, const std::vector<std::string> &prefixes);

        /**
         * Delete all relevant triples from the database, while showing a progress bar on std::cout
         */
        void deleteTriplesFromDatabase();

        /**
         * Send SPARQL queries to delete all triples that belong to the nodes that are inserted to
         * the database
         */
        void deleteNodesFromDatabase(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send SPARQL queries to delete all triples that belong to the ways that are inserted to
         * the database
         */
        void deleteWaysFromDatabase(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send SPARQL queries to delete meta-data and key-value triples that belong to the ways
         * that did not change their geometry
         */
        void deleteWaysMetaDataAndTags(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
        * Send SPARQL queries to delete all triples that belong to the relations that are inserted to
        * the database
        */
        void deleteRelationsFromDatabase(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send SPARQL queries to insert all relevant triples
         */
        void insertTriplesToDatabase();

        /**
         * Filters the triples that where generated by osm2rdf. Relevant triples are triples for osm
         * elements that occurred in the change file or osm elements which geometry needs to be
         * updated. Irrelevant triples are triples that where generated for referenced elements.
         */
        std::vector<Triple> filterRelevantTriples();

        /**
         * Returns the elements id.
         *
         * Example: For a node element with id 1787 the function would return '1787'
         *
         * @param element The osm element
         * @return The id of the element
         */
        static id_t getIdFor(const boost::property_tree::ptree &element);

        /**
         * Returns the elements location.
         *
         * @param element The osm element
         * @return The location of the element
         */
        static osmium::Location getLocationFor(const boost::property_tree::ptree &element);

        static member_ids_t getMemberFor(const boost::property_tree::ptree &element);
    };

    /**
     * Exception that can appear inside the `OsmChangeHandler` class.
     */
    class OsmChangeHandlerException final : public std::exception {
        std::string message;
    public:
        explicit OsmChangeHandlerException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::osm

#endif //OSM_LIVE_UPDATES_OSMCHANGEHANDLER_H
