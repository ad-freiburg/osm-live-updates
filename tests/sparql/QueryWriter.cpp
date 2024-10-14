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
    TEST(QueryWriter, insertQueries) {
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
    TEST(QueryWriter, deleteQueries) {
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
    TEST(QueryWriter, deleteNodeQuery) {
        {
            std::string query = olu::sparql::QueryWriter::writeNodeDeleteQuery(1);
            ASSERT_EQ(
                    "DELETE WHERE { osmnode:1 ?p0 ?o0 . osm2rdfgeom:osm_node_1 ?p1 ?o1 . }",
                    query
            );
        }
    }
    TEST(QueryWriter, deleteWayQuery) {
        {
            std::string query = olu::sparql::QueryWriter::writeWayDeleteQuery(1);
            ASSERT_EQ(
                    "DELETE WHERE { osmway:1 ?p0 ?o0 . osm2rdf:way_1 ?p1 ?o1 . }",
                    query
            );
        }
    }
    TEST(QueryWriter, deleteRelationQuery) {
        {
            std::string query = olu::sparql::QueryWriter::writeRelationDeleteQuery(1);
            ASSERT_EQ(
                    "DELETE WHERE { osmrel:1 ?p0 ?o0 . }",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForNodeLocation) {
        {
            int nodeId = 1;

            std::string query = olu::sparql::QueryWriter::writeQueryForNodeLocation(nodeId);
            ASSERT_EQ(
                    "SELECT ?o WHERE { osm2rdfgeom:osm_node_1 geo:asWKT ?o . }",
                    query
            );
        }
    }
}
