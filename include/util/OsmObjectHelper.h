//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include "util/Types.h"
#include "osm/ChangeAction.h"

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <osmium/osm/object.hpp>

namespace olu::osm {
    class OsmObjectHelper {
    public:

        /**
         * @return True if the given relation is of type "multipolygon"
         */
        static bool isMultipolygon(const boost::property_tree::ptree &relation);

        static id_t getIdFromUri(const std::string& uri);
        static std::string getOsmTagFromUri(const std::string& uri);

        static bool areWayMemberEqual(member_ids_t member1, member_ids_t member2);
        static bool areRelMemberEqual(rel_members_t member1, rel_members_t member2);

        /**
         * To check whether an object has been created or modified, we check if the version is
         * equal to 1. This is, however, not always correct, as it can happen that we merge several
         * change files. An object created in one file may have been modified in a later one but is
         * still missing on the sparql endpoint, e.g., it is equal to a create operation for the
         * endpoint.
         *
         * @param osmObject The osm object to get the action for.
         * @return The action (create, modify or delete) that was performed on the given osm element
         * in the change file
         */
        static ChangeAction getChangeAction(const osmium::OSMObject &osmObject);
    };

    /**
     * Exception that can appear inside the `WktHelper` class.
     */
    class OsmObjectHelperException final : public std::exception {
        std::string message;

    public:
        explicit OsmObjectHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };
}
#endif //OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
