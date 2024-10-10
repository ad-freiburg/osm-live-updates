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
        explicit OsmChangeHandler(config::Config& config)
        : _config(config), _sparql(config), _osm2ttl(), _odf(OsmDataFetcher(config)) { }

        /**
         * @brief Processes an osm change file.
         *
         * Reads the contents of an osm change file, loops over the changesets with `create`,
         * `modify` or `delete` tag and processes the contained osm elements
         *
         * @param pathToOsmChangeFile The path to the change file
         * @param deleteChangeFile If `True` the change file will be deleted after finishing
         * @param showProgress Show a progress bar in the command line
         */
        void handleChange(const std::string &pathToOsmChangeFile,
                          const bool &deleteChangeFile,
                          const bool &showProgress = true);

        /**
         * Counts the osm elements (`node`, `way` or `relation`) in the osm change file that are in
         * contained in the changesets.
         *
         * @param osmChangeElement the osm change element
         * @return The number of elements to be processed
         */
        static size_t countElements(const boost::property_tree::ptree &osmChangeElement);

        /**
         * Returns a vector with all osm elements needed for conversion to ttl format. For `nodes`
         * and `relation`, this is only the passed element. If the element is a `way` all the nodes
         * referenced in this way are fetched and added to the vector.
         *
         * The first entry of the returned vector is always: `<osm version="0.6">` and the last one
         * `</osm>`
         *
         * @param elementTag The tag of the element
         * @param element The xml element of to get the informations for
         * @return A vector containig all informations needed for the conversion to ttl
         */
        std::vector<std::string> getOsmElementsForInsert(const std::string& elementTag,
                                                         const pt::ptree& element);

        /**
         * Creates a SPARQL query from the given ttl data to add the contained triples into the
         * database and sends it to the SPARQL endpoint.
         *
         * @param convertedData The ttl data containing the triples that should be inserted to the datatbase
         * as well as the needed prefixes
         */
        void createAndRunInsertQuery(const std::vector<std::string> &convertedData,
                                     const std::string &elementTag,
                                     const pt::ptree &element);

        /**
         * Filters the prefixes from the converted ttl data and formats it for SPARQL:
         *
         * e.g. `@prefix ohmnode: <https://www.openhistoricalmap.org/node/> .` is formatted to
         * `PREFIX ohmnode: <https://www.openhistoricalmap.org/node/> .`
         *
         * @param ttl the ttl data form the conversion
         * @return A vector with all prefixes in the conversion data formatted for SPARQL
         */
        static std::vector<std::string> getPrefixesFromConvertedData(std::vector<std::string> ttl);

        /**
         * Filters the triples from the converted ttl data. For Ways all triples that result from
         * from the node references that where needed for conversion are also filtered out.
         *
         * @param ttl the ttl data form the conversion
         * @return A vector with all triples in the conversion data
         */
        static std::vector<std::string> getTriplesFromConvertedData(std::vector<std::string> ttl,
                                                             const std::string &elementTag,
                                                             const pt::ptree &element);

    private:
        config::Config _config;
        sparql::SparqlWrapper _sparql;
        Osm2ttl _osm2ttl;
        OsmDataFetcher _odf;
        Stats _stats;

        /**
         * @brief Handles the insertion of elements.
         *
         *  Inserts the provided element into the database, by converting the xml element to ttl
         *  triples and updating the sparql endpoint.
         *
         * @param elementTag The tag of the element to insert, which can either be `node`, `way` or
         * `relation`
         * @param element The xml element to insert
         */
        void handleInsert(const std::string& elementTag, const pt::ptree& element);

        /**
         * @brief Handles the deletion of elements.
         *
         *  Deletes the provided xml element from the database, by getting the subject of the
         *  element and sending a sparql query to the endpoint which deletes all triples containing
         *  the subject
         *
         * @param elementTag The tag of the element to delete, which can either be `node`, `way` or
         * `relation`
         * @param element The xml element to delete
         */
        void handleDelete(const std::string& elementTag, const pt::ptree& element);

        /**
         * @brief Handles the modification of elements.
         *
         *  Modifies the provided element in the database, by deleting the old data and inserting
         *  new element
         *
         * @param elementTag The tag of the element to modify, which can either be `node`, `way` or
         * `relation`
         * @param element The xml element to modify
         */
        void handleModify(const std::string& elementTag, const pt::ptree& element);

        /**
         * Gets the id of each node that is referenced in the passed `way` element
         *
         * @param way The `way` element to get the ids of all referenced nodes
         * @return A vector containing the ids of all referenced nodes
         */
        static std::vector<long long> getIdsOfReferencedNodes(const boost::property_tree::ptree &way);

        /**
         * Creates an vector containing dummy nodes for the given node ids. The dummy nodes contain
         * the node id and the location which is used for the nodes that are referenced in ways.
         *
         * @param nodeIds The node ids to create dummy nodes for
         * @return A vector containing a dummy node for each given node id
         */
        std::vector<std::string> createDummyNodes(const std::vector<long long>& nodeIds);

        /**
         * @brief Handles the deletion of members of a relation.
         *
         * @param relationId The realtion osm element for wich the members should be deleted
         */
        void handleRelationMemberDeletion(const long long &relationId);
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
