//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

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
