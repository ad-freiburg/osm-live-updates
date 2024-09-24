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
            std::string subject = "osmrel:1960198";

            std::string query = olu::sparql::QueryWriter::writeDeleteQuery(subject);
            ASSERT_EQ(
                    "DELETE { ?s ?p ?o } WHERE { osmrel:1960198 ?p ?o . }",
                    query
            );
        }
    }
    TEST(QueryWriter, writeQueryForNodeLocation) {
        {
            int nodeId = 1;

            std::string query = olu::sparql::QueryWriter::writeQueryForNodeLocation(nodeId);
            ASSERT_EQ(
                    "SELECT ?o WHERE { "
                    "osmnode:1 geo:hasGeometry/geo:asWKT ?o . "
                    "}",
                    query
            );
        }
    }
}
