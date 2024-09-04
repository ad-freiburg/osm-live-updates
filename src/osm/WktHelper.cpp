//
// Created by Nicolas von Trott on 28.08.24.
//

#include "osm/WktHelper.h"

#include <boost/regex.hpp>
#include <string>

std::string olu::osm::WktHelper::createDummyNodeFromPoint(const std::string& nodeId,
                                                          const std::string& pointAsWkt) {
    const boost::regex pattern(R"(POINT\(([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)\))");
    boost::smatch match;

    std::string lat;
    std::string lon;
    if (boost::regex_search(pointAsWkt, match, pattern)) {
        lon = match[1];
        lat = match[2];
    } else {
        throw WktHelperException(("No WKT Point found in " + pointAsWkt).c_str());
    }

    return "<node id=\"" + nodeId + "\" lat=\"" + lat + "\" lon=\"" + lon + "\"/>";
}
