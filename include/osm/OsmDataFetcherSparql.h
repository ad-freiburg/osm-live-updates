//
// Created by Nicolas von Trott on 27.05.25.
//

#ifndef OSMDATAFETCHERSPARQL_H
#define OSMDATAFETCHERSPARQL_H

#include <set>
#include <string>

#include "simdjson.h"
#include "StatisticsHandler.h"

#include "osm/OsmDataFetcher.h"
#include "osm/Node.h"
#include "osm/Relation.h"
#include "osm/Way.h"
#include "sparql/SparqlWrapper.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"

namespace olu::osm {
    class OsmDataFetcherSparql final : public OsmDataFetcher {
    public:
        explicit OsmDataFetcherSparql(const config::Config &config, StatisticsHandler &stats)
            : _config(config), _stats(&stats), _sparqlWrapper(config), _queryWriter(config) { }

        std::vector<Node> fetchNodes(const std::set<id_t> &nodeIds) override;

        void fetchAndWriteNodesToFile(const std::string &filePath, const std::set<id_t> &nodeIds) override;

        size_t fetchAndWriteRelationsToFile(const std::string &filePath, const std::set<id_t> &relationIds) override;

        size_t fetchAndWriteWaysToFile(const std::string &filePath, const std::set<id_t> &wayIds) override;

        member_ids_t fetchWaysMembers(const std::set<id_t> &wayIds) override;

        std::pair<std::vector<id_t>, std::vector<id_t>>
        fetchRelationMembers(const std::set<id_t> &relIds) override;

        std::string fetchLatestTimestamp() override;

        std::vector<id_t> fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) override;

        std::vector<id_t> fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) override;

        std::vector<id_t> fetchRelationsReferencingWays(const std::set<id_t> &wayIds) override;

        std::vector<id_t>
        fetchRelationsReferencingRelations(const std::set<id_t> &relationIds) override;

        std::string fetchOsm2RdfVersion() override;

        std::map<std::string, std::string> fetchOsm2RdfOptions() override;

        OsmDatabaseState fetchUpdatesCompleteUntil() override;

        std::string fetchReplicationServer() override;
    private:
        config::Config _config;
        StatisticsHandler* _stats;
        sparql::SparqlWrapper _sparqlWrapper;
        sparql::QueryWriter _queryWriter;
        simdjson::ondemand::parser _parser;

        simdjson::padded_string runQuery(const std::string &query,
                                         const std::vector<std::string> &prefixes);

        /**
         * Parses the items in a list that is delimited by ";" and applies the given function to
         * each item in the list.
         * @tparam T The return type of the function that is applied to each item in the list
         * @param list The list of items that are delimited by ";"
         * @param function The function to apply to each item in the list
         * @return A vector containing the manipulated items of the list.
         */
        template <typename T> std::vector<T>
        parseValueList(const std::string_view &list, std::function<T(std::string)> function);

        /**
         * Returns the JSON element at "results.bindings" for the given document.
         */
        static simdjson::simdjson_result<simdjson::ondemand::value> getBindings(
            simdjson::simdjson_result<simdjson::ondemand::document> &doc);

        /**
         * Returns the string at the "value" element for the given JSON element.
         */
        template <typename T> static T getValue(
            simdjson::simdjson_result<simdjson::ondemand::value> value);
    };
}
#endif //OSMDATAFETCHERSPARQL_H
