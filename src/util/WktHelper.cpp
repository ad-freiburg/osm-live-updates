//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/WktHelper.h"

#include <boost/regex.hpp>
#include <string>
#include <iostream>

std::string olu::osm::WktHelper::createDummyNodeFromPoint(const long long &nodeId,
                                                          const std::string& pointAsWkt) {
    const boost::regex pattern(R"(POINT\(([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)\))");
    boost::smatch match;

    std::string lat;
    std::string lon;
    if (boost::regex_search(pointAsWkt, match, pattern)) {
        lon = match[1];
        lat = match[2];
    } else {
        std::string message = "No WKT Point found in " + pointAsWkt;
        throw WktHelperException(message.c_str());
    }

    return "<node id=\"" + std::to_string(nodeId) + "\" lat=\"" + lat + "\" lon=\"" + lon + "\"/>";
}

std::pair<double, double> olu::osm::WktHelper::getLatLonFromPoint(const std::string &pointAsWkt) {
    const boost::regex pattern(R"(POINT\(([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)\))");
    boost::smatch match;

    std::string latStr;
    std::string lonStr;
    if (boost::regex_search(pointAsWkt, match, pattern)) {
        lonStr = match[1];
        latStr = match[2];
    } else {
        std::string message = "No WKT Point found in " + pointAsWkt;
        throw WktHelperException(message.c_str());
    }

    double lat = -1.0;
    double lon = -1.0;
    try {
        lat = std::stod(latStr);
        lon = std::stod(lonStr);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::string msg = "Could not get Location from WKT Point " + pointAsWkt;
        throw WktHelperException(msg.c_str());
    }

    return {lat, lon};
}
