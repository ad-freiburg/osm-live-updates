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

#ifndef OSM_LIVE_UPDATES_SPARQLWRAPPER_H
#define OSM_LIVE_UPDATES_SPARQLWRAPPER_H

#include "util/HttpMethod.h"

#include <string>
#include <vector>

namespace olu::sparql {

class SparqlWrapper {
public:
    explicit SparqlWrapper(std::string& endpointUri);

    // Sets the HTTP Method for the query. Typically, `SELECT` queries should be performed with
    // `GET` and update queries (`DELETE`, `INSERT`) with `POST`
    void setMethod(util::HttpMethod method);

    void setQuery(std::string& query);

    std::string runQuery();
private:
    std::string _endpoint;
    util::HttpMethod _httpMethod = util::POST;
    std::string _query;
};

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_SPARQLWRAPPER_H
