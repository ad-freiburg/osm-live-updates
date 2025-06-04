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

            _modifiedRelations.insert(relation.id());
            _stats->countModifiedRelation();
    }
}
