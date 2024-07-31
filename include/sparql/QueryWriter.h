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

#ifndef OSM_LIVE_UPDATES_QUERYWRITER_H
#define OSM_LIVE_UPDATES_QUERYWRITER_H

#include <string>
#include <vector>

namespace olu::sparql {

class QueryWriter {
public:
    // Writes a SPARQL query that inserts a list of triples
    static std::string writeInsertQuery(std::vector<std::string>& triples);

    // Writes a SPARQL query that deletes all triples for a subject
    static std::string writeDeleteQuery(std::string& subject);
};

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_QUERYWRITER_H
