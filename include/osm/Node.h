// Copyright 2024, University of Freiburg
// Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

// This file is part of osm-live-updates.
//
// osm-live-updates is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm-live-updates is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM_LIVE_UPDATES_NODE_H
#define OSM_LIVE_UPDATES_NODE_H

#include <utility>
#include "string"

#include "util/Types.h"
#include "osmium/osm/location.hpp"

namespace olu::osm {

    class Node {
    public:
        explicit Node(u_id id, const WKTPoint& locationAsWkt);

        /**
         * Returns the node as an xml osm object.
         *
         * @example For a node with id: `1` and location: `POINT(13.5690032 42.7957187)` the function
         * would return: `<node id="1" lat="42.7957187" lon="13.5690032"/>`
         */
        std::string get_xml();

        osmium::Location get_location() { return loc; };
        [[nodiscard]] u_id get_id() const { return id; };
    protected:
        u_id id;
        osmium::Location loc;
    };

    /**
     * Exception that can appear inside the `Node` class.
     */
    class NodeException : public std::exception {
    private:
        std::string message;
    public:
        explicit NodeException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //OSM_LIVE_UPDATES_NODE_H
