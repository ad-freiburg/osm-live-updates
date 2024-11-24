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

    typedef std::vector<u_id> WayMembers;

}

#endif //OSM_LIVE_UPDATES_TYPES_H
