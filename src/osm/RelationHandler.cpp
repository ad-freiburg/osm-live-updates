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

#include "osm/RelationHandler.h"

#include <iostream>

#include "osmium/osm/relation.hpp"
#include "osm2rdf/util/Time.h"

#include "config/Constants.h"
#include "osm/OsmObjectHelper.h"

namespace cnst = olu::config::constants;

// _________________________________________________________________________________________________
void olu::osm::RelationHandler::relation(const osmium::Relation& relation) {
    switch (OsmObjectHelper::getChangeAction(relation)) {
        case ChangeAction::CREATE:
            _createdRelations.insert(relation.id());
            _stats->countCreatedRelation();
            break;
        case ChangeAction::DELETE:
            _deletedRelations.insert(relation.id());
            _stats->countDeletedRelation();
            break;
        case ChangeAction::MODIFY:
            const auto *const typeTag = relation.tags()["type"];
            if (typeTag != nullptr && (strcmp(typeTag, "multipolygon") == 0 ||
                                       strcmp(typeTag, "boundary") == 0)) {
                _modifiedAreas.insert(relation.id());
            }

            std::vector<RelationMember> members;
            for (const auto &member: relation.members()) {
                members.emplace_back(member.ref(), member.type(),member.role());
            }

            _modifiedRelationsBuffer.emplace(relation.id(), members);
            _stats->countModifiedRelation();
    }
}

// _________________________________________________________________________________________________
void olu::osm::RelationHandler::checkRelationsForMemberChange(
    const std::set<id_t> &modifiedNodesWithChangedLocation,
    const std::set<id_t> &modifiedWaysWithChangedMembers) {

    for (const auto &[relId, relMembers] : _modifiedRelationsBuffer) {
        if (memberWasModifiedInChangeFile(relId, relMembers,
                                          modifiedNodesWithChangedLocation,
                                          modifiedWaysWithChangedMembers)) {
            continue;
        }

        const auto membersEndpoint = _odf->fetchRelsMembersSorted({relId});
        if (membersEndpoint.empty()) {
            _createdRelations.insert(relId);
            _stats->switchModifiedToCreatedRelation();
            continue;
        }

        for (const auto &[relId, memberEndpoint]: membersEndpoint) {
            if (RelationMember::areRelMemberEqual(relMembers, memberEndpoint)) {
                _modifiedRelations.insert(relId);
            } else {
                _modifiedRelationsWithChangedMembers.insert(relId);
                _stats->countRelationWithMemberChange();
            }
        }
    }
}

// _________________________________________________________________________________________________
bool olu::osm::RelationHandler::memberWasModifiedInChangeFile(
    const id_t &relationId,
    const std::vector<RelationMember> &members,
    const std::set<id_t> &modifiedNodesWithChangedLocation,
    const std::set<id_t> &modifiedWaysWithChangedMembers) {

    bool hasModifiedMember = false;
    for (const auto &member : members) {
        if (member.type == OsmObjectType::NODE) {
            if (modifiedNodesWithChangedLocation.contains(member.id)) {
                _modifiedRelationsWithChangedMembers.insert(relationId);
                hasModifiedMember = true;
                break;
            }
        } else if (member.type == OsmObjectType::WAY) {
            if (modifiedWaysWithChangedMembers.contains(member.id)) {
                _modifiedRelationsWithChangedMembers.insert(relationId);
                hasModifiedMember = true;
                break;
            }
        } else if (member.type == OsmObjectType::RELATION) {
            // At the moment, all relations that have a relation as member are handled
            // as if their geometry has changed.
            // This is not ideal, but to be sure that the geometry hasn't changed,
            // we would have to check for all members that are relations if their geometry has
            // changed, but this is not known at this point.
            _modifiedRelationsWithChangedMembers.insert(relationId);
            hasModifiedMember = true;
            break;
        }
    }

    return hasModifiedMember;
}
