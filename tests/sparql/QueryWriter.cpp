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

#include "sparql/QueryWriter.h"
#include "config/Constants.h"
#include "gtest/gtest.h"
#include "osm2rdf/config/Config.h"

namespace olu::sparql {
    TEST(QueryWriter, writeDeleteOsmObjectQuery) {
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?value ?p ?o . } "
                    "WHERE { VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value ?p ?o . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?value ?p ?o . } "
                    "WHERE { VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value ?p ?o . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?value ?p ?o . } "
                    "WHERE { VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value ?p ?o . }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?value ?p ?o . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value ?p ?o . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?value ?p ?o . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value ?p ?o . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?value ?p ?o . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value ?p ?o . } }",
                    query
            );
        }
    }

    TEST(QueryWriter, writeDeleteOsmObjectGeometryQuery) {
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectGeometryQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value geo:hasGeometry ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
    }

    TEST(QueryWriter, writeDeleteOsmObjectCentroidQuery) {
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { ?o geo:asWKT ?geometry . } "
                    "WHERE { VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::NODE, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::WAY, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteOsmObjectCentroidQuery(osm::OsmObjectType::RELATION, {1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { ?o geo:asWKT ?geometry . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value geo:hasCentroid ?o . "
                    "?o geo:asWKT ?geometry . } }",
                    query
            );
        }
    }

    TEST(QueryWriter, writeDeleteWayMemberQuery) {
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteWayMemberQuery({1, 2, 3});
            ASSERT_EQ(
                    "DELETE { "
                    "?o osmway:member_id ?member_id . "
                    "?o osmway:member_pos ?member_pos . } "
                    "WHERE { VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value osmway:member ?o . "
                    "?o osmway:member_id ?member_id . "
                    "?o osmway:member_pos ?member_pos . "
                    "}",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteWayMemberQuery({1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { "
                    "?o osmway:member_id ?member_id . "
                    "?o osmway:member_pos ?member_pos . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value osmway:member ?o . "
                    "?o osmway:member_id ?member_id . "
                    "?o osmway:member_pos ?member_pos . } }",
                    query
            );
        }
    }

    TEST(QueryWriter, writeDeleteRelMemberQuery) {
        {
            const QueryWriter qw{config::Config()};
            const std::string query = qw.writeDeleteRelMemberQuery({1, 2, 3});
            ASSERT_EQ(
                    "DELETE { "
                    "?o osmrel:member_id ?member_id . "
                    "?o osmrel:member_pos ?member_pos . "
                    "?o osmrel:member_role ?member_role . } "
                    "WHERE { VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value osmrel:member ?o . "
                    "?o osmrel:member_id ?member_id . "
                    "?o osmrel:member_pos ?member_pos . "
                    "?o osmrel:member_role ?member_role . "
                    "}",
                    query
            );
        }
        {
            config::Config config {};
            config.graphUri = "https://example.org/a";
            const QueryWriter qw{config};
            const std::string query = qw.writeDeleteRelMemberQuery({1, 2, 3});
            ASSERT_EQ(
                    "DELETE { GRAPH <https://example.org/a> { "
                    "?o osmrel:member_id ?member_id . "
                    "?o osmrel:member_pos ?member_pos . "
                    "?o osmrel:member_role ?member_role . } } "
                    "WHERE { GRAPH <https://example.org/a> { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value osmrel:member ?o . "
                    "?o osmrel:member_id ?member_id . "
                    "?o osmrel:member_pos ?member_pos . "
                    "?o osmrel:member_role ?member_role . } }",
                    query
            );
        }
    }

    TEST(QueryWriter, writeInsertQuery) {
        {
            std::vector<std::string> triples;
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440");

            QueryWriter qw{config::Config()};
            std::string query = qw.writeInsertQuery(triples);
            ASSERT_EQ(
                    "osmrel:1960198 ogc:sfContains ?osm_id:10559440 . ",
                    query
            );
        }
        {
            std::vector<std::string> triples;
            triples.emplace_back("osmrel:1960198 ogc:sfContains ?osm_id:10559440");
            triples.emplace_back("region:102740 osmkey:name name:Bretagne");

            QueryWriter qw{config::Config()};
            std::string query = qw.writeInsertQuery(triples);
            ASSERT_EQ(
                    "osmrel:1960198 ogc:sfContains ?osm_id:10559440 . "
                    "region:102740 osmkey:name name:Bretagne . ",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForNodeLocations) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForNodeLocations({1, 2, 3});
            ASSERT_EQ(
            "SELECT ?value ?location "
            "WHERE { VALUES ?value { osm2rdfgeom:osm_node_1 osm2rdfgeom:osm_node_2 osm2rdfgeom:osm_node_3 } "
            "?value geo:asWKT ?location . }",
            query
            );
        }
    }
    TEST(QueryWriter, writeQueryForLatestNodeTimestamp) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForLatestNodeTimestamp();
            ASSERT_EQ(
                    "SELECT ?timestamp WHERE { ?node rdf:type osm:node . ?node osmmeta:timestamp ?timestamp . } "
                    "ORDER BY DESC(?timestamp) LIMIT 1",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForWaysMembers) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForReferencedNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?node WHERE { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?value osmway:member ?member . "
                    "?member osmway:member_id ?node . } "
                    "GROUP BY ?node",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationMembersWay) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForRelationMemberIds({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?member WHERE { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?value osmrel:member ?o . "
                    "?o osmrel:member_id ?member . } "
                    "GROUP BY ?member",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForWaysReferencingNodes) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForWaysReferencingNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?way WHERE { "
                    "VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?s osmway:member_id ?value . "
                    "?way osmway:member ?s . } "
                    "GROUP BY ?way",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingNodes) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForRelationsReferencingNodes({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?rel WHERE { "
                    "VALUES ?value { osmnode:1 osmnode:2 osmnode:3 } "
                    "?rel osmrel:member ?o . "
                    "?o osmrel:member_id ?value . "
                    "} GROUP BY ?rel",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingWays) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForRelationsReferencingWays({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?rel WHERE { "
                    "VALUES ?value { osmway:1 osmway:2 osmway:3 } "
                    "?rel osmrel:member ?o . "
                    "?o osmrel:member_id ?value . "
                    "} GROUP BY ?rel",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForRelationsReferencingRelations) {
        {
            QueryWriter qw{config::Config()};
            std::string query = qw.writeQueryForRelationsReferencingRelations({1, 2, 3});
            ASSERT_EQ(
                    "SELECT ?s WHERE { "
                    "VALUES ?value { osmrel:1 osmrel:2 osmrel:3 } "
                    "?s osmrel:member ?o . "
                    "?o osmrel:member_id ?value . "
                    "} GROUP BY ?s",
                    query
            );
        }
    }
}
