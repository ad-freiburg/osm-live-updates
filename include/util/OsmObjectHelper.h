//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include <string>
#include <set>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "osm/Node.h"

namespace olu::osm {
    class OsmObjectHelper {
    public:

        /**
         *
         * @param osmElement
         * @return True if the given relation is of type "multipolygon"
         */
        static bool isMultipolygon(const boost::property_tree::ptree &relation);
    };

    /**
     * Exception that can appear inside the `WktHelper` class.
     */
    class WktHelperException : public std::exception {
    private:
        std::string message;

    public:
        explicit WktHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };
}
#endif //OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
