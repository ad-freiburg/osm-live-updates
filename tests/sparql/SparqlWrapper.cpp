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

#include "config/Constants.h"
#include "gtest/gtest.h"
#include "config/Config.h"
#include "sparql/SparqlWrapper.h"

namespace cnst = olu::config::constants;

namespace olu::sparql {
    TEST(SparqlWrapper, emptyQuery) {
        auto config((olu::config::Config()));
        config.sparqlEndpointUri = "https://qlever.cs.uni-freiburg.de/api/osm-planet";
        auto sparqlWrapper((olu::sparql::SparqlWrapper(config)));

        ASSERT_THROW(sparqlWrapper.runQuery(), olu::sparql::SparqlWrapperException);
    }

    TEST(SparqlWrapper, nonEmptyResponseFromEndpoint) {
        auto config((olu::config::Config()));
        config.sparqlEndpointUri = "https://qlever.cs.uni-freiburg.de/api/osm-planet";
        auto sparqlWrapper((olu::sparql::SparqlWrapper(config)));
        sparqlWrapper.setPrefixes({"PREFIX osmnode: <https://www.openstreetmap.org/node/>"});
        sparqlWrapper.setQuery("SELECT * WHERE { osmnode:1 ?p ?o }");

        ASSERT_FALSE(sparqlWrapper.runQuery().empty());
    }
}