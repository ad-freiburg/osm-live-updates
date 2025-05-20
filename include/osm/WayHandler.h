// Copyright 2025, University of Freiburg
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

#ifndef WAYHANDLER_H
#define WAYHANDLER_H

#include <osmium/handler.hpp>
#include <map>
#include <set>

#include "OsmDataFetcher.h"

namespace olu::osm {

    class WayHandler: public osmium::handler::Handler {
    public:
        explicit WayHandler(const config::Config &config): _config(config), _odf(config) {}

        // Iterator for osmium::apply
        void way(const osmium::Way& way);

        [[nodiscard]] std::set<id_t> getCreatedWays() const { return _createdWays; }
        [[nodiscard]] std::set<id_t> getModifiedWays() const { return _modifiedWays; }
        [[nodiscard]] std::set<id_t> getDeletedWays() const { return _deletedWays; }

        [[nodiscard]] std::set<id_t> getModifiedWaysWithChangedMembers() const {
            return _modifiedWaysWithChangedMembers; }

        void printWayStatistics() const;

        /**
         * Checks if the members of the given ways from the change file have changed. If so, the
         * way is added to the _modifiedWaysWithChangedMembers set, otherwise to the
         * _modifiedWays set
         *
         * @param modifiedNodesWithChangedLocation The ids of modified nodes with a changed location
         */
        void checkWaysForMemberChange(std::set<id_t> modifiedNodesWithChangedLocation);

        /**
         * @return True if the change file contains no ways.
         */
        bool empty() const {
            return _createdWays.empty() &&
                   _modifiedWays.empty() &&
                   _modifiedWaysWithChangedMembers.empty() &&
                   _deletedWays.empty();
        }

        /**
         * @Returns TRUE if the way with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All ways inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is inside the loop over the osm
         * elements inside `storeIdsOfElementsInChangeFile()` after the first way has occured.
         */
        [[nodiscard]] bool wayInChangeFile(const id_t &wayId) const {
            return _modifiedWays.contains(wayId) ||
                   _modifiedWaysWithChangedMembers.contains(wayId) ||
                   _createdWays.contains(wayId) ||
                   _deletedWays.contains(wayId);
        }

    private:
        config::Config _config;
        OsmDataFetcher _odf;

        // Ways that are in a delete-changeset in the change file.
        std::set<id_t> _deletedWays;
        // Ways that are in a create-changeset in the change file.
        std::set<id_t> _createdWays;

        // Buffer to store the members of the ways that are in a modify-changeset in the change file.
        std::map<id_t, member_ids_t> _modifiedWaysBuffer;
        // Ways that are in a modify-changeset in the change file and not have a changed member list
        std::set<id_t> _modifiedWays;
        // Ways that are in a modify-changeset in the change file and have a changed member list.
        std::set<id_t> _modifiedWaysWithChangedMembers;
    };

}
    
#endif //WAYHANDLER_H
