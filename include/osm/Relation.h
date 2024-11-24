//
// Created by Nicolas von Trott on 24.11.24.
//

#ifndef RELATION_H
#define RELATION_H

#include <string>
#include <set>

#include "util/Types.h"

namespace olu::osm {

    class Relation {
    public:
        explicit Relation(const u_id id): id(id) {};

        void setType(std::string const& type);

        void addNodeAsMember(u_id const& id, Role const& role);
        void addWayAsMember(u_id const& id, Role const& role);
        void addRelationAsMember(u_id const& id, Role const& role);

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

        std::set<RelationMember> getNodeMembers() { return nodes; };
        std::set<RelationMember> getWayMembers() { return ways; };
        std::set<RelationMember> getRelationMembers() { return relations; };
        [[nodiscard]] u_id getId() const { return id; };
    protected:
        u_id id;
        std::string type;
        std::set<RelationMember> nodes;
        std::set<RelationMember> ways;
        std::set<RelationMember> relations;
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
