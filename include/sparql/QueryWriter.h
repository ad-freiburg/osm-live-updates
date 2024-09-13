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
#include <boost/property_tree/ptree.hpp>

namespace olu::sparql {

class QueryWriter {
public:
    // Returns the elements subject formatted for a SPARQL query:
    // For a node element with id 178709586 the function would return 'osmnode:178709586'
    static std::string getSubjectFor(const std::string& elementTag,
                                     const boost::property_tree::ptree &element);

    // Returns a SPARQL query that inserts a list of triples
    static std::string writeInsertQuery(const std::vector<std::string>& triples);

    // Returns a SPARQL query that deletes all triples for a subject
    static std::string writeDeleteQuery(const std::string& subject);

    // Returns a SPARQL query that asks for the location of a point in WKT format
    static std::string writeQueryForNodeLocation(const std::string& nodeId);

    // Returns a SPARQL query that asks for the latest timestamp of any node in the database
    static std::string writeQueryForLatestNodeTimestamp();
};

} // namespace olu::sparql

#endif //OSM_LIVE_UPDATES_QUERYWRITER_H
