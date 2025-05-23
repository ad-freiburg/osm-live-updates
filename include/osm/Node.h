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

#include "string"

#include "osmium/osm/location.hpp"

#include "util/Types.h"

namespace olu::osm {

    class Node {
    public:
        explicit Node(id_t id, const wktPoint_t& locationAsWkt);
        explicit Node(id_t id, const osmium::Location& location);

        /**
         * Returns the node as a XML osm object.
         *
         * @example For a node with id: `1` and location: `POINT(13.5690032 42.7957187)` the function
         * would return: `<node id="1" lat="42.7957187" lon="13.5690032"/>`
         */
        [[nodiscard]] std::string getXml() const;

        [[nodiscard]] osmium::Location getLocation() const { return loc; };
        [[nodiscard]] id_t getId() const { return id; };
    protected:
        id_t id;
        osmium::Location loc;
    };

    /**
     * Exception that can appear inside the `Node` class.
     */
    class NodeException final : public std::exception {
        std::string message;
    public:
        explicit NodeException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //OSM_LIVE_UPDATES_NODE_H
