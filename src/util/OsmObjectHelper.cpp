//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/OsmObjectHelper.h"
#include "config/Constants.h"
#include "util/XmlReader.h"
#include <string>

namespace cnst = olu::config::constants;

bool olu::osm::OsmObjectHelper::isMultipolygon(const boost::property_tree::ptree &relation) {
    for (const auto &result: relation.get_child("")) {
        if (result.first == "tag") {
            if (olu::util::XmlReader::readAttribute("<xmlattr>.k",
                                                    result.second) == "type" &&
                olu::util::XmlReader::readAttribute("<xmlattr>.v",
                                                    result.second) == "multipolygon") {
             return true;
            }
        }
    }
    return false;
}