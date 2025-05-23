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

#include "osm/ReferencesHandler.h"

#include "osm2rdf/osm/Relation.h"
#include "osmium/osm/way.hpp"

#include "util/BatchHelper.h"

// _________________________________________________________________________________________________
void olu::osm::ReferencesHandler::way(const osmium::Way &way) {
    for (const auto& node : way.nodes()) {
        if (!_nodeHandler.nodeInChangeFile(node.ref())) {
            _referencedNodes.insert(node.ref());
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::ReferencesHandler::relation(const osmium::Relation& relation) {
    for (const auto& member : relation.members()) {
        switch (member.type()) {
            case osmium::item_type::node:
                if (!_nodeHandler.nodeInChangeFile(member.ref())) {
                    _referencedNodes.insert(member.ref());
                }
                break;
            case osmium::item_type::way:
                if (!_wayHandler.wayInChangeFile(member.ref())) {
                    _referencedWays.insert(member.ref());
                }
                break;
            case osmium::item_type::relation:
                if (!_relationHandler.relationInChangeFile(member.ref())) {
                    _referencedRelations.insert(member.ref());
                }
                break;
            default:
                const std::string msg = "Cannot handle type for member with id " +
                                        std::to_string(member.ref()) + "for relation with id "
                                        + std::to_string(relation.id());
                throw ReferencesHandlerException(msg.c_str());
        }
    }
}

// _________________________________________________________________________________________________
void olu::osm::ReferencesHandler::getReferencesForRelations(const std::set<id_t> &relationIds) {
    if (!relationIds.empty()) {
        util::BatchHelper::doInBatches(
                relationIds,
                _config.maxValuesPerQuery,
                [this](const std::set<id_t>& batch) {
                auto [nodeIds, wayIds] = _odf.fetchRelationMembers(batch);
                for (const auto &wayId: wayIds) {
                    _referencedWays.insert(wayId);
                }
                for (const auto &nodeId: nodeIds) {
                    _referencedNodes.insert(nodeId);
                }
            });
    }
}

// _________________________________________________________________________________________________
void olu::osm::ReferencesHandler::getReferencesForWays(const std::set<id_t> &wayIds) {
    if (!wayIds.empty()) {
        util::BatchHelper::doInBatches(
        wayIds,
        _config.maxValuesPerQuery,
        [this](const std::set<id_t>& batch) {
            for (const auto &nodeId: _odf.fetchWaysMembers(batch)) {
                _referencedNodes.insert(nodeId);
            }
        });
    }
}