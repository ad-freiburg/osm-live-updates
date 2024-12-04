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

#include <boost/property_tree/ptree.hpp>
#include "sparql/QueryWriter.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

namespace olu::sparql {
    TEST(QueryWriter, writeInsertQuery) {
        {
            std::vector<std::string> triples;
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440");

            std::string query = olu::sparql::QueryWriter::writeInsertQuery(triples);
            ASSERT_EQ(
                    "INSERT DATA { "
                    "osmrel:1960198 ogc:sfContains ?osm_id:10559440 . "
                    "}",
                    query
            );
        }
        {
            std::vector<std::string> triples;
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440");
            triples.emplace_back("region:102740 osmkey:name name:Bretagne");

            std::string query = olu::sparql::QueryWriter::writeInsertQuery(triples);
            ASSERT_EQ(
                    "INSERT DATA { "
                    "osmrel:1960198 ogc:sfContains ?osm_id:10559440 . "
                    "region:102740 osmkey:name name:Bretagne . "
                    "}",
                    query
            );
        }
    }
    TEST(QueryWriter, writeDeleteQuery) {
        {
            std::string query = QueryWriter::writeDeleteQuery({1960198, 1960199},
            "osmnode");
            ASSERT_EQ(
                    "DELETE { ?s ?p1 ?o1 . ?o1 ?p2 ?o2 . } "
                    "WHERE { VALUES ?s { osmnode:1960198 osmnode:1960199 } "
                    "?s ?p1 ?o1 . "
                    "OPTIONAL { ?o1 ?p2 ?o2. } }",
                    query
            );
        }

        {
            std::string query = QueryWriter::writeDeleteQuery({1960199},
            "osmway");
            ASSERT_EQ(
                    "DELETE { ?s ?p1 ?o1 . ?o1 ?p2 ?o2 . } "
                    "WHERE { VALUES ?s { osmway:1960199 } "
                    "?s ?p1 ?o1 . "
                    "OPTIONAL { ?o1 ?p2 ?o2. } }",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForNodeLocations) {
        {
            const std::string query = QueryWriter::writeQueryForNodeLocations({1, 2, 3});
            ASSERT_EQ(
            "SELECT ?nodeGeo ?location "
            "WHERE { VALUES ?nodeGeo { osm2rdfgeom:osm_node_1 osm2rdfgeom:osm_node_2 osm2rdfgeom:osm_node_3 } "
            "?nodeGeo geo:asWKT ?location . }",
            query
            );
        }
    }
    TEST(QueryWriter, writeQueryForLatestNodeTimestamp) {
        {
            std::string query = QueryWriter::writeQueryForLatestNodeTimestamp();
            ASSERT_EQ(
                    "SELECT ?p WHERE { ?s rdf:type osm:node . ?s osmmeta:timestamp ?p . } "
                    "ORDER BY DESC(?p) LIMIT 1",
                    query
            );
        }
    }
    // TEST(QueryWriter, writeQueryForRelationMembers) {
    //     {
    //         std::string query = olu::sparql::QueryWriter::writeQueryForRelations(1);
    //         ASSERT_EQ(
    //                 "SELECT ?id ?role ?key WHERE { "
    //                 "osmrel:1 osmkey:type ?key . "
    //                 "osmrel:1 osmrel:member ?o . "
    //                 "?o osm2rdfmember:id ?id . "
    //                 "?o osm2rdfmember:role ?role . "
    //                 "}",
    //                 query
    //         );
    //     }
    // }
    // TEST(QueryWriter, writeQueryForWayMembers) {
    //     {
    //         std::string query = olu::sparql::QueryWriter::writeQueryForWayMembers({1});
    //         ASSERT_EQ(
    //                 "SELECT ?node WHERE { "
    //                 "osmway:1 osmway:node ?member . "
    //                 "?member osmway:node ?node ."
    //                 "}",
    //                 query
    //         );
    //     }
    // }
    TEST(QueryWriter, writeQueryForWaysMembers) {
        {
            std::string query = QueryWriter::writeQueryForReferencedNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?node WHERE { "
                    "VALUES ?way { osmway:1 osmway:2 osmway:3 } "
                    "?way osmway:node ?member . "
                    "?member osmway:node ?node . } "
                    "GROUP BY ?node",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationMembersWay) {
        {
            std::string query = QueryWriter::writeQueryForRelationMembers({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?p WHERE { "
                    "VALUES ?rel { osmrel:1 osmrel:2 osmrel:3 } "
                    "?rel osmrel:member ?o . "
                    "?o osm2rdfmember:id ?p . } "
                    "GROUP BY ?p",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForWaysReferencingNodes) {
        {
            std::string query = QueryWriter::writeQueryForWaysReferencingNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?way WHERE { "
                    "VALUES ?node { osmnode:1 osmnode:2 osmnode:3 } "
                    "?identifier osmway:node ?node . "
                    "?way osmway:node ?identifier . } "
                    "GROUP BY ?way",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingNodes) {
        {
            std::string query = QueryWriter::writeQueryForRelationsReferencingNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "VALUES ?node { osmnode:1 osmnode:2 osmnode:3 } "
                    "?s osmrel:member ?o . "
                    "?o osm2rdfmember:id ?node . "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingWays) {
        {
            std::string query = QueryWriter::writeQueryForRelationsReferencingWays({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "VALUES ?way { osmway:1 osmway:2 osmway:3 } "
                    "?s osmrel:member ?o . "
                    "?o osm2rdfmember:id ?way . "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingRelations) {
        {
            std::string query = QueryWriter::writeQueryForRelationsReferencingRelations({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "VALUES ?rel { osmrel:1 osmrel:2 osmrel:3 } "
                    "?s osmrel:member ?o . "
                    "?o osm2rdfmember:id ?rel . "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
}
