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

#include "util/TtlHelper.h"

#include <iostream>
#include <string>
#include <string_view>

#include "config/Constants.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
olu::triple_t olu::util::TtlHelper::parseTriple(const std::string& tripleString) {
    std::string_view tripleView = tripleString;

    // Trim trailing dot and space
    if (tripleView.ends_with(" .")) {
        tripleView.remove_suffix(2);
    }

    try {
        // Parse subject
        const size_t pos1 = tripleView.find(' ');
        const std::string_view subject = tripleView.substr(0, pos1);
        tripleView.remove_prefix(pos1 + 1);

        const size_t pos2 = tripleView.find(' ');
        const std::string_view predicate = tripleView.substr(0, pos2);
        tripleView.remove_prefix(pos2 + 1);

        // The remaining triple view is the object.
        return std::make_tuple(std::string(subject),
                               std::string(predicate),
                               std::string(tripleView));
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Invalid triple format: " + tripleString;
        throw TtlHelperException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
olu::id_t olu::util::TtlHelper::parseId(const std::string &prefixedName) {
    const size_t end = prefixedName.size();
    size_t start = end;

    try {
        // Scan backward to find the start of the last number
        while (start > 0 && std::isdigit(static_cast<unsigned char>(prefixedName[start - 1]))) {
            --start;
        }

        return stoll(prefixedName.substr(start, end - start));
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        const std::string msg = "Invalid prefixed name: " + prefixedName;
        throw TtlHelperException(msg.c_str());
    }
}

// _________________________________________________________________________________________________
bool olu::util::TtlHelper::isInNamespaceForOsmObject(const std::string& subject,
                                                     const osm::OsmObjectType & osmObject) {
    switch (osmObject) {
        case osm::OsmObjectType::NODE:
            return subject.starts_with(cnst::NAMESPACE_OSM_NODE);
        case osm::OsmObjectType::WAY:
            return subject.starts_with(cnst::NAMESPACE_OSM_WAY);
        case osm::OsmObjectType::RELATION:
            return subject.starts_with(cnst::NAMESPACE_OSM_REL);
    }

    const std::string msg = "Cant interpret subject: " + subject;
    throw TtlHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
bool olu::util::TtlHelper::isMetadataOrTagPredicate(const std::string &predicate,
                                                    const osm::OsmObjectType &osmObject) {
    switch (osmObject) {
        case osm::OsmObjectType::NODE:
            throw TtlHelperException("Node subjects should not be checked for metadata or "
                                     "tag predicates");
        case osm::OsmObjectType::WAY:
            return predicate.starts_with(cnst::NAMESPACE_OSM_KEY) ||
                   predicate.starts_with(cnst::NAMESPACE_OSM_META) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_FACTS);
        case osm::OsmObjectType::RELATION:
            return predicate.starts_with(cnst::NAMESPACE_OSM_KEY) ||
                   predicate.starts_with(cnst::NAMESPACE_OSM_META) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_FACTS);
    }

    const std::string msg = "Cant interpret predicate for metadata and tag check: " + predicate;
    throw TtlHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
bool olu::util::TtlHelper::isGeometryPredicate(const std::string &predicate,
                                               const osm::OsmObjectType &osmObject) {
    switch (osmObject) {
        case osm::OsmObjectType::NODE:
            throw TtlHelperException("Node subjects should not be checked for geometry "
                                     "predicates");
        case osm::OsmObjectType::WAY:
            return predicate.starts_with(cnst::NAMESPACE_OSM2RDF_GEOM) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_LENGTH) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_AREA);
        case osm::OsmObjectType::RELATION:
            return predicate.starts_with(cnst::NAMESPACE_OSM2RDF_GEOM) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_LENGTH) ||
                   predicate.starts_with(cnst::PREFIXED_OSM2RDF_AREA);
    }

    const std::string msg = "Cant interpret predicate for geometry check: " + predicate;
    throw TtlHelperException(msg.c_str());
}

// _________________________________________________________________________________________________
bool olu::util::TtlHelper::hasRelevantObject(const std::string& predicate,
                                             const osm::OsmObjectType & osmObject) {
    switch (osmObject) {
        case osm::OsmObjectType::NODE:
            return predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
        case osm::OsmObjectType::WAY:
            return predicate == cnst::PREFIXED_WAY_MEMBER ||
                   predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
        case osm::OsmObjectType::RELATION:
            return predicate == cnst::PREFIXED_REL_MEMBER ||
                   predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
    }

    const std::string msg = "Cant interpret predicate for relevancy check: " + predicate;
    throw TtlHelperException(msg.c_str());
}