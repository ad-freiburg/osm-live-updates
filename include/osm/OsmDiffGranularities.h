//
// Created by Nicolas von Trott on 26.07.24.
//

#ifndef OSM_LIVE_UPDATES_OSMDIFFGRANULARITIES_H
#define OSM_LIVE_UPDATES_OSMDIFFGRANULARITIES_H

#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>

using boost::assign::map_list_of;

enum OsmDiffGranularity {
    MINUTE, HOUR, DAY
};

const boost::unordered_map<OsmDiffGranularity,const char*> urlSegmentFor = map_list_of
        (MINUTE, "minute")
        (HOUR, "hour")
        (DAY, "day");

#endif //OSM_LIVE_UPDATES_OSMDIFFGRANULARITIES_H
