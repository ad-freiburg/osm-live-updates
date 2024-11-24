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
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440 .");

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
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440 .");
            triples.emplace_back("region:102740 osmkey:name name:Bretagne .");

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
            std::vector<std::string> subjects;
            subjects.emplace_back("osmrel:1960198");

            std::string query = olu::sparql::QueryWriter::writeDeleteQuery(subjects);
            ASSERT_EQ(
                    "DELETE WHERE { osmrel:1960198 ?p0 ?o0 . }",
                    query
            );
        }

        {
            std::vector<std::string> subjects;
            subjects.emplace_back("osmrel:1960198");
            subjects.emplace_back("osmnode:1");

            std::string query = olu::sparql::QueryWriter::writeDeleteQuery(subjects);
            ASSERT_EQ(
                    "DELETE WHERE { osmrel:1960198 ?p0 ?o0 . osmnode:1 ?p1 ?o1 . }",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForNodeLocations) {
        {
            std::set<long long> nodeIds {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForNodeLocations(nodeIds);
            ASSERT_EQ(
                    "SELECT ?s ?o WHERE { { BIND (osm2rdfgeom:osm_node_1 AS ?s ) . ?s geo:asWKT ?o . } "
                    "UNION { BIND (osm2rdfgeom:osm_node_2 AS ?s ) . ?s geo:asWKT ?o . } "
                    "UNION { BIND (osm2rdfgeom:osm_node_3 AS ?s ) . ?s geo:asWKT ?o . } }",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForLatestNodeTimestamp) {
        {
            std::string query = olu::sparql::QueryWriter::writeQueryForLatestNodeTimestamp();
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
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForReferencedNodes(ids);
            ASSERT_EQ(
                    "SELECT ?node WHERE { "
                    "{ osmway:1 osmway:node ?member . ?member osmway:node ?node . } "
                    "UNION { osmway:2 osmway:node ?member . ?member osmway:node ?node . } "
                    "UNION { osmway:3 osmway:node ?member . ?member osmway:node ?node . } "
                    "} GROUP BY ?node",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationMembersWay) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForRelationMembers(ids);
            ASSERT_EQ(
                    "SELECT ?p WHERE { "
                    "{ osmrel:1 osmrel:member ?o . ?o osm2rdfmember:id ?p . ?p rdf:type osm:way } "
                    "UNION { osmrel:2 osmrel:member ?o . ?o osm2rdfmember:id ?p . ?p rdf:type osm:way } "
                    "UNION { osmrel:3 osmrel:member ?o . ?o osm2rdfmember:id ?p . ?p rdf:type osm:way } "
                    "} GROUP BY ?p",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForWaysReferencingNodes) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForWaysReferencingNodes(ids);
            ASSERT_EQ(
                    "SELECT ?s2 WHERE { "
                    "{ ?s1 osmway:node osmnode:1 . ?s2 osmway:node ?s1 . } "
                    "UNION { ?s1 osmway:node osmnode:2 . ?s2 osmway:node ?s1 . } "
                    "UNION { ?s1 osmway:node osmnode:3 . ?s2 osmway:node ?s1 . } "
                    "} GROUP BY ?s2",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingNodes) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingNodes(ids);
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "{ ?s osmrel:member ?o0 . ?o0 osm2rdfmember:id osmnode:1 . } "
                    "UNION { ?s osmrel:member ?o1 . ?o1 osm2rdfmember:id osmnode:2 . } "
                    "UNION { ?s osmrel:member ?o2 . ?o2 osm2rdfmember:id osmnode:3 . } "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingWays) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingWays(ids);
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "{ ?s osmrel:member ?o0 . ?o0 osm2rdfmember:id osmway:1 . } "
                    "UNION { ?s osmrel:member ?o1 . ?o1 osm2rdfmember:id osmway:2 . } "
                    "UNION { ?s osmrel:member ?o2 . ?o2 osm2rdfmember:id osmway:3 . } "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingRelations) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeQueryForRelationsReferencingRelations(ids);
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "{ ?s osmrel:member ?o0 . ?o0 osm2rdfmember:id osmrel:1 . } "
                    "UNION { ?s osmrel:member ?o1 . ?o1 osm2rdfmember:id osmrel:2 . } "
                    "UNION { ?s osmrel:member ?o2 . ?o2 osm2rdfmember:id osmrel:3 . } "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
    TEST(QueryWriter, writeNodesDeleteQuery) {
        {
            std::set<long long> ids {1, 2, 3} ;
            std::string query = olu::sparql::QueryWriter::writeNodesDeleteQuery(ids);
            ASSERT_EQ(
                    "DELETE WHERE { "
                    "osmnode:1 ?p0 ?o0 . osm2rdfgeom:osm_node_1 ?p1 ?o1 . "
                    "osmnode:2 ?p2 ?o2 . osm2rdfgeom:osm_node_2 ?p3 ?o3 . "
                    "osmnode:3 ?p4 ?o4 . osm2rdfgeom:osm_node_3 ?p5 ?o5 . }",
                    query
            );
        }
    }

}
