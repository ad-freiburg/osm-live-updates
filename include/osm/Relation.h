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

#ifndef RELATION_H
#define RELATION_H

#include "util/Types.h"

#include <string>
#include <utility>
#include <vector>

namespace olu::osm {
    class Relation {
    public:
        explicit Relation(const id_t id): id(id) {};

        void setType(std::string const& type);
        void setTimestamp(std::string const& timestamp);
        void setVersion(version_t const& version);
        void setChangesetId(changeset_id_t const& changeset_id);

        void addMember(const RelationMember& member);
        void addTag(const std::string& key, const std::string& value);

        /**
         * Returns an osm xml relation with an id and members.
         *
         * @example For relationId: `1` and members: `{
         * ("https://www.openstreetmap.org/node/1", "amin_centre"),
         * ("https://www.openstreetmap.org/way/1", "outer"),
         * ("https://www.openstreetmap.org/relation/1", "inner")}` the
         * function would return: `<relation id="1">
         * <member type="node" ref="1" role="amin_centre">
         * <member type="way" ref="1"  role="outer">
         * <member type="relation" ref="1"  role="inner">
         * </relation>`
         */
        [[nodiscard]] std::string getXml() const;

        rel_members_t getMembers() { return members; }
        [[nodiscard]] id_t getId() const { return id; }
        std::vector<key_value_t> getTags() { return tags; }
        std::string getTimestamp() { return timestamp; }
        [[nodiscard]] version_t getVersion() const { return version; }
        [[nodiscard]] changeset_id_t getChangesetId() const { return changeset_id; }
    protected:
        id_t id;
        std::string timestamp;
        version_t version = 0;
        changeset_id_t changeset_id = 0;
        std::string type;
        rel_members_t members;
        std::vector<key_value_t> tags;
    };

    /**
     * Exception that can appear inside the `Node` class.
     */
    class RelationException final : public std::exception {
        std::string message;
    public:
        explicit RelationException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //RELATION_H
