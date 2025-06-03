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

#ifndef RELATIONHANDLER_H
#define RELATIONHANDLER_H

#include <map>
#include <set>

#include "StatisticsHandler.h"
#include "osmium/handler.hpp"

#include "osm/OsmDataFetcher.h"

namespace olu::osm {
    class RelationHandler: public osmium::handler::Handler {
    public:
        explicit RelationHandler(const config::Config &config, OsmDataFetcher &odf,
                                 StatisticsHandler &stats): _config(config), _odf(&odf),
                                                            _stats(&stats) {}

        // Iterator for osmium::apply
        void relation(const osmium::Relation& relation);

        [[nodiscard]] std::set<id_t> getCreatedRelations() const { return _createdRelations; }
        [[nodiscard]] std::set<id_t> getModifiedRelations() const { return _modifiedRelations; }
        [[nodiscard]] std::set<id_t> getModifiedAreas() const { return _modifiedAreas; }
        [[nodiscard]] std::set<id_t> getDeletedRelations() const { return _deletedRelations; }

        [[nodiscard]] std::set<id_t> getModifiedRelationsWithChangedMembers() const {
            return _modifiedRelationsWithChangedMembers;}
        [[nodiscard]] std::set<id_t> getAllRelations() const {
            std::set<id_t> allRelations;
            allRelations.insert(_createdRelations.begin(), _createdRelations.end());
            allRelations.insert(_modifiedRelations.begin(), _modifiedRelations.end());
            allRelations.insert(_modifiedRelationsWithChangedMembers.begin(),
                                _modifiedRelationsWithChangedMembers.end());
            allRelations.insert(_deletedRelations.begin(), _deletedRelations.end());
            return allRelations;
        }
        [[nodiscard]] size_t getNumOfRelations() const {
            return _createdRelations.size() +
                   _modifiedRelations.size() +
                   _modifiedRelationsWithChangedMembers.size() +
                   _deletedRelations.size();
        }

        /**
         * Checks if the members of the given relations from the change file have changed.
         */
        void
        checkRelationsForMemberChange(const std::set<id_t> &modifiedNodesWithChangedLocation,
                                      const std::set<id_t>& modifiedWaysWithChangedMembers);

        /**
         * Returns true if one of the members of the relation with the given ID was modified in the
         * change file, in a way, that the relation geometry is affected.
         * If that is the case, the relation id is inserted in the
         * _modifiedRelationsWithChangedMembers set
         *
         * @param relationId The id of the relation to check the members for.
         * @param members The members of the relation.
         * @param modifiedNodesWithChangedLocation The set with the ids of all nodes in the change
         * file that have a modified location
         * @param modifiedWaysWithChangedMembers The set with the ids of all ways in the change
         * file that have a modified member list
         * @return True if one of the members of the relation was modified in the change file.
         */
        bool memberWasModifiedInChangeFile(const id_t &relationId,
                                           const std::vector<RelationMember> &members,
                                           const std::set<id_t> &modifiedNodesWithChangedLocation,
                                           const std::set<id_t> &modifiedWaysWithChangedMembers);

        /**
         * @return True if the change file contains no relations.
         */
        [[nodiscard]] bool empty() const {
            return _createdRelations.empty() &&
                   _modifiedRelations.empty() &&
                   _modifiedRelationsWithChangedMembers.empty() &&
                   _deletedRelations.empty();
        }

        /**
         * @Returns TRUE if the relation with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All relations inside the ChangeFile have to be processed BEFORE using this function.
         * Therefore, the earliest time this function can be called is after calling
         * `storeIdsOfElementsInChangeFile()`
         */
        [[nodiscard]] bool relationInChangeFile(const id_t &relationId) const {
            return _modifiedRelations.contains(relationId) ||
                   _modifiedRelationsWithChangedMembers.contains(relationId) ||
                   _createdRelations.contains(relationId) ||
                   _deletedRelations.contains(relationId);
        }

    private:
        config::Config _config;
        OsmDataFetcher* _odf;
        StatisticsHandler* _stats;

        // Relations that are in a delete-changeset in the change file.
        std::set<id_t> _deletedRelations;
        // Relations that are in a create-changeset in the change file.
        std::set<id_t> _createdRelations;

        // Buffer to store the members of the relations that are in a modify-changeset in the change file.
        std::map<id_t, std::vector<RelationMember>> _modifiedRelationsBuffer;
        // Relations that are in a modify-changeset in the change file and not have a changed member
        std::set<id_t> _modifiedRelations;
        // Relations that are in a modify-changeset in the change file and have a changed member list.
        std::set<id_t> _modifiedRelationsWithChangedMembers;
        // Relations that are of type multipolygon that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedAreas;
    };
}

#endif //RELATIONHANDLER_H
