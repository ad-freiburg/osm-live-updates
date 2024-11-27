//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/OsmObjectHelper.h"
#include "util/XmlReader.h"

#include <string>

bool olu::osm::OsmObjectHelper::isMultipolygon(const boost::property_tree::ptree &relation) {
    for (const auto &[tag, attr]: relation.get_child("")) {
        if (tag == "tag") {
            if (util::XmlReader::readAttribute("<xmlattr>.k", attr) == "type" &&
                util::XmlReader::readAttribute("<xmlattr>.v", attr) == "multipolygon") {
                return true;
            }
        }
    }

    return false;
}