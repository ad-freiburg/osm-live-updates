//
// Created by Nicolas von Trott on 27.05.25.
//

#ifndef OSMDATAFETCHERQLEVER_H
#define OSMDATAFETCHERQLEVER_H
#include <set>
#include <string>

#include "simdjson.h"

#include "osm/OsmDataFetcher.h"
#include "osm/Node.h"
#include "osm/Relation.h"
#include "osm/Way.h"
#include "sparql/SparqlWrapper.h"
#include "sparql/QueryWriter.h"
#include "util/Types.h"

namespace olu::osm {
    class OsmDataFetcherQLever final : public OsmDataFetcher {
    public:
        explicit OsmDataFetcherQLever(const config::Config &config)
            : _config(config), _sparqlWrapper(config), _queryWriter(config) { }

        std::vector<Node> fetchNodes(const std::set<id_t> &nodeIds) override;

        std::vector<Relation> fetchRelations(const std::set<id_t> &relationIds) override;

        void fetchRelationInfos(Relation &relation) override;

        std::vector<Way> fetchWays(const std::set<id_t> &wayIds) override;

        void fetchWayInfos(Way &way) override;

        member_ids_t fetchWaysMembers(const std::set<id_t> &wayIds) override;

        std::vector<std::pair<id_t, member_ids_t>>
        fetchWaysMembersSorted(const std::set<id_t> &wayIds) override;

        std::vector<std::pair<id_t, std::vector<RelationMember>>>
        fetchRelsMembersSorted(const std::set<id_t> &relIds) override;

        std::pair<std::vector<id_t>, std::vector<id_t>>
        fetchRelationMembers(const std::set<id_t> &relIds) override;

        std::string fetchLatestTimestampOfAnyNode() override;

        std::vector<id_t> fetchWaysReferencingNodes(const std::set<id_t> &nodeIds) override;

        std::vector<id_t> fetchRelationsReferencingNodes(const std::set<id_t> &nodeIds) override;

        std::vector<id_t> fetchRelationsReferencingWays(const std::set<id_t> &wayIds) override;

        std::vector<id_t>
        fetchRelationsReferencingRelations(const std::set<id_t> &relationIds) override;
    private:
        config::Config _config;
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
         * Iterates over the results of the given response and applies the given function to each
         * JSON element in "res".
         */
        void forResults(const simdjson::padded_string &response,
                        std::function<void(simdjson::ondemand::value)> func);

        /**
         * Returns the JSON element at "results.bindings" for the given document.
         */
        static simdjson::ondemand::value getResults(simdjson::ondemand::document &doc);

        /**
         * Returns the string at the "value" element for the given JSON element.
         */
        template <typename T> static T getValue(simdjson::ondemand::value value);
    };
}
#endif //OSMDATAFETCHERQLEVER_H
