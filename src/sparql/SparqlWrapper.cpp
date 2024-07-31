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

#include "sparql/SparqlWrapper.h"
#include "util/URLHelper.h"
#include "util/HttpRequest.h"

#include <string>

namespace olu::sparql {

// _________________________________________________________________________________________________
SparqlWrapper::SparqlWrapper(std::string &endpointUri) {
    _endpoint = endpointUri;
}

// _________________________________________________________________________________________________
void SparqlWrapper::setMethod(util::HttpMethod httpMethod) {
    _httpMethod = httpMethod;
}

// _________________________________________________________________________________________________
void SparqlWrapper::setQuery(std::string &query) {
    _query = query;
}

// _________________________________________________________________________________________________
std::string SparqlWrapper::runQuery() {
    std::string encodedQuery = util::URLHelper::encodeForUrlQuery(_query);
    std::string url = _endpoint + "?" + encodedQuery;

    auto request = util::HttpRequest(_httpMethod,url);

    if (_httpMethod == util::POST) {
        request.addHeader("Content-Type", "application/sparql-query");
    }

    return request.perform();
}

} // namespace olu::sparql
