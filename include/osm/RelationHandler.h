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
        [[nodiscard]] std::set<id_t> getAllRelations() const {
            std::set<id_t> allRelations;
            allRelations.insert(_createdRelations.begin(), _createdRelations.end());
            allRelations.insert(_modifiedRelations.begin(), _modifiedRelations.end());
            allRelations.insert(_deletedRelations.begin(), _deletedRelations.end());
            return allRelations;
        }
        [[nodiscard]] size_t getNumOfRelations() const {
            return _createdRelations.size() +
                   _modifiedRelations.size() +
                   _deletedRelations.size();
        }

        /**
         * @return True if the change file contains no relations.
         */
        [[nodiscard]] bool empty() const {
            return _createdRelations.empty() &&
                   _modifiedRelations.empty() &&
                   _deletedRelations.empty();
        }

        /**
         * @Returns TRUE if the relation with the given ID is contained in a `create`, `modify` or
         * 'delete' changeset in the changeFile.
         *
         * @warning All relations inside the ChangeFile have to be processed BEFORE using this
         * function. Therefore, the earliest time this function can be called is after calling
         * `storeIdsOfElementsInChangeFile()`
         */
        [[nodiscard]] bool relationInChangeFile(const id_t &relationId) const {
            return _modifiedRelations.contains(relationId) ||
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
        // Relations that are in a modify-changeset in the change file and not have a changed member
        std::set<id_t> _modifiedRelations;
        // Relations that are of type multipolygon that are in a modify-changeset in the change file.
        std::set<id_t> _modifiedAreas;
    };
}

#endif //RELATIONHANDLER_H
