//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include "util/Types.h"

#include <string>
#include <set>
#include <boost/property_tree/ptree.hpp>

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
