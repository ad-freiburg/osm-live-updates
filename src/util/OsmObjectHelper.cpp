//
// Created by Nicolas von Trott on 28.08.24.
//

#include "util/OsmObjectHelper.h"
#include "util/XmlReader.h"

#include <iostream>
#include <string>
#include <config/Constants.h>

namespace cnst = olu::config::constants;
namespace olu::osm {
    bool OsmObjectHelper::isMultipolygon(const boost::property_tree::ptree &relation) {
        for (const auto &[tag, attr]: relation.get_child("")) {
            if (tag == cnst::XML_TAG_TAG) {
                if (util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_KEY, attr) == "type" &&
                    util::XmlReader::readAttribute<std::string>(cnst::XML_PATH_ATTR_VALUE, attr) == "multipolygon") {
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

    std::string OsmObjectHelper::getOsmTagFromUri(const std::string& uri) {
        if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_NODE)) {
            return cnst::XML_TAG_NODE;
        }

        if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_WAY)) {
            return cnst::XML_TAG_WAY;
        }

        if (uri.starts_with(cnst::NAMESPACE_IRI_OSM_REL)) {
            return cnst::XML_TAG_REL;
        }

        const std::string msg = "Cant extract osm tag from uri: " + uri;
        throw OsmObjectHelperException(msg.c_str());
    }

    bool OsmObjectHelper::areWayMemberEqual(member_ids_t member1, member_ids_t member2) {
        return member1.size() == member2.size() &&
            std::equal(member1.begin(), member1.end(), member2.begin());
    }

    bool OsmObjectHelper::areRelMemberEqual(rel_members_t member1, rel_members_t member2) {
        return member1.size() == member2.size() &&
            std::equal(member1.begin(), member1.end(), member2.begin(),
                     [](const RelationMember& a, const RelationMember& b) {
                         return a.id == b.id &&
                                a.osmTag == b.osmTag &&
                                a.role == b.role;
                     });
    }

    ChangeAction OsmObjectHelper::getChangeAction(const osmium::OSMObject &osmObject) {
        if (osmObject.deleted()) { return ChangeAction::DELETE; }
        if (osmObject.version() == 1) { return ChangeAction::CREATE; }
        return ChangeAction::MODIFY;
    }
}


