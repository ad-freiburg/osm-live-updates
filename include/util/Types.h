//
// Created by Nicolas von Trott on 22.11.24.
//

#ifndef OSM_LIVE_UPDATES_TYPES_H
#define OSM_LIVE_UPDATES_TYPES_H

#include <cstdint>
#include <vector>

namespace olu {

    typedef int64_t id;
    typedef uint64_t u_id;

    typedef std::string WKTPoint;
    typedef std::string Role;

    typedef std::vector<u_id> WayMembers;
    typedef std::pair<u_id, Role> RelationMember;
}

#endif //OSM_LIVE_UPDATES_TYPES_H
