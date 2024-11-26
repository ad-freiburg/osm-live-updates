//
// Created by Nicolas von Trott on 22.11.24.
//

#ifndef OSM_LIVE_UPDATES_TYPES_H
#define OSM_LIVE_UPDATES_TYPES_H

#include <cstdint>
#include <vector>

namespace olu {

    typedef int64_t id_t;

    typedef std::string WKTPoint;
    typedef std::string Role;

    typedef std::vector<id_t> WayMembers;
    typedef std::pair<id_t, Role> RelationMember;
}

#endif //OSM_LIVE_UPDATES_TYPES_H
