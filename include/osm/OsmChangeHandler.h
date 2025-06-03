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

#include <set>

#include "osmium/handler.hpp"
#include "osm2rdf/util/ProgressBar.h"

#include "config/Config.h"
#include "osm/OsmDataFetcher.h"
#include "osm/NodeHandler.h"
#include "osm/ReferencesHandler.h"
#include "osm/RelationHandler.h"
#include "osm/WayHandler.h"
#include "sparql/SparqlWrapper.h"
#include "sparql/QueryWriter.h"
#include "osm/StatisticsHandler.h"

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
    class OsmChangeHandler: public osmium::handler::Handler {
    public:
        explicit OsmChangeHandler(const config::Config &config, OsmDataFetcher &odf,
                                  StatisticsHandler &stats);
        void run();
    private:
        config::Config _config;
        sparql::SparqlWrapper _sparql;
        sparql::QueryWriter _queryWriter;
        OsmDataFetcher* _odf;
        StatisticsHandler* _stats;

        /**
         * Osmium handler for the nodes in the change file.
         * Sorts the ids of the nodes into the respective sets (_createdNodes,
         * _modifiedNodes/_modifiedNodesWithChangedLocation, _deletedNodes).
         */
        NodeHandler _nodeHandler;

        /**
         * Osmium handler for the ways in the change file.
         * Sorts the ids of the ways into the respective sets (_createdWays,
         * _modifiedWays/_modifiedWaysWithChangedMembers, _deletedWays).
         */
        WayHandler _wayHandler;

        /**
         * Osmium handler for the relations in the change file.
         * Sorts the ids of the relations into the respective sets (_createdRelations,
         * _modifiedRelations/_modifiedRelationsWithChangedMembers, _deletedRelations).
         */
        RelationHandler _relationHandler;

        /**
         * Osmium handler for the references (member nodes, ways and relations) in the change file.
         * Sorts the ids of the references into the respective sets (_referencedNodes,
         * _referencedWays, _referencedRelations).
         */
        ReferencesHandler _referencesHandler;


        // Ways that reference a node which was modified in the changeset.
        std::set<id_t> _waysToUpdateGeometry;
        // Relations that reference a node, way or relation which was modified in the changeset.
        std::set<id_t> _relationsToUpdateGeometry;

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
        void createDummyNodes(osm2rdf::util::ProgressBar &progress,
                              size_t &counter);

        /**
         * Creates dummy ways for the referenced ways that are not in the change file and writes
         * them to a temporary file The dummy ways only contain the referenced nodes
         */
        void createDummyWays(osm2rdf::util::ProgressBar &progress,
                             size_t &counter);

        /**
         * Creates dummy relations for the referenced relations that are not in the change file and
         * writes them to a temporary file.
         * The dummy relation only contains the members of that relation
         */
        void createDummyRelations(
            osm2rdf::util::ProgressBar &progress, size_t &counter);

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
         * Send SPARQL queries to delete geometry triples that belong to the ways for which only the
         * geometry changed
         */
        void deleteWaysGeometry(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
        * Send SPARQL queries to delete all triples that belong to the relations that are inserted to
        * the database
        */
        void deleteRelationsFromDatabase(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send SPARQL queries to delete meta-data and key-value triples that belong to the
         * relations that did not change their member and therefore their geometry
         */
        void deleteRelationsMetaDataAndTags(osm2rdf::util::ProgressBar &progress, size_t &counter);

        /**
         * Send SPARQL queries to delete geometry triples that belong to the ways for which only the
         * geometry changed
         */
        void deleteRelationsGeometry(osm2rdf::util::ProgressBar &progress, size_t &counter);


        /**
         * Send SPARQL queries to insert all relevant triples
         */
        void insertTriplesToDatabase();

        /**
         * Filters the triples that where generated by osm2rdf. Relevant triples are triples for osm
         * elements that occurred in the change file or osm elements which geometry needs to be
         * updated. Irrelevant triples are triples that where generated for referenced elements.
         */
        [[nodiscard]] std::vector<triple_t> filterRelevantTriples() const;

        /**
         * Checks if the given triple is relevant for the osm node object it belongs to and adds it
         * to the relevantTriples vector if that is the case.
         */
        static void filterNodeTriple(const triple_t &nodeTriple, const std::set<id_t> &nodesToInsert,
                              std::vector<triple_t> &relevantTriples,
                              std::string &currentLink);

        /**
         * Checks if the given triple is relevant for the osm way object it belongs to and adds it
         * to the relevantTriples vector if that is the case.
         */
        void filterWayTriple(const triple_t &wayTriple, const std::set<id_t> &waysToInsert,
                             std::vector<triple_t> &relevantTriples,
                             std::string &currentLink) const;

        /**
         * Checks if the given triple is relevant for the osm relation object it belongs to and
         * adds it to the relevantTriples vector if that is the case.
         */
        void filterRelationTriple(const triple_t &relationTriple,
                                                 const std::set<id_t> &relationsToInsert,
                                                 std::vector<triple_t> &relevantTriples,
                                                 std::string &currentLink) const;

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
