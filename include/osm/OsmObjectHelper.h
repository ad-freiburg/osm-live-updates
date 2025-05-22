//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include <string>

#include "osmium/osm/object.hpp"

#include "osm/OsmObjectType.h"
#include "osm/ChangeAction.h"
#include "util/Types.h"

namespace olu::osm {
    class OsmObjectHelper {
    public:
        /**
         * Parses the id from an uri like "https://www.openstreetmap.org/node/1" or
         * "https://osm2rdf.cs.uni-freiburg.de/rdf/geom#osm_node_10916447545".
         *
         * The uri has to end with the id number.
         * There is no validation of the input,
         * so make sure it is in the correct format.
         *
         * @param uri The uri to extract the id from.
         * @return The extracted id as an id_t type.
         */
        static id_t parseIdFromUri(const std::string_view &uri);

        static OsmObjectType parseOsmTypeFromUri(const std::string& uri);

        /**
         * Returns true if the two lists of way members are equal.
         * This is the case if the members in both lists are exactly the same and in the same order
         */
        static bool areWayMemberEqual(member_ids_t member1, member_ids_t member2);

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
