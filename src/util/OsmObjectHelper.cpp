//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/OsmObjectHelper.h"
#include "util/XmlReader.h"

#include <iostream>
#include <string>

namespace olu::osm {
    bool OsmObjectHelper::isMultipolygon(const boost::property_tree::ptree &relation) {
        for (const auto &[tag, attr]: relation.get_child("")) {
            if (tag == "tag") {
                if (util::XmlReader::readAttribute<std::string>("<xmlattr>.k", attr) == "type" &&
                    util::XmlReader::readAttribute<std::string>("<xmlattr>.v", attr) == "multipolygon") {
                    return true;
                    }
            }
        }

        return false;
    }

    id_t OsmObjectHelper::getIdFromUri(const std::string &uri) {
        std::vector<char> id;
        // Read characters from end of uri until first non digit is reached
        for (auto it = uri.rbegin(); it != uri.rend(); ++it) {
            if (std::isdigit(*it)) {
                id.push_back(*it);
            } else {
                break;
            }
        }

        try {
            return std::stoll(std::string(id.rbegin(), id.rend()));
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            const std::string msg = "Cant extract id from uri: " + uri;
            throw OsmObjectHelperException(msg.c_str());
        }
    }

    bool OsmObjectHelper::areMemberEqual(member_ids_t member1, member_ids_t member2) {
        return member1.size() == member2.size() &&
            std::equal(member1.begin(), member1.end(), member2.begin());
    }

}


