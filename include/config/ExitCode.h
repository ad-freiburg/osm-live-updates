//
// Created by Nicolas von Trott on 17.09.24.
//

#ifndef OSM_LIVE_UPDATES_EXITCODE_H
#define OSM_LIVE_UPDATES_EXITCODE_H

namespace olu::config {

    enum ExitCode {
        SUCCESS = 0,
        FAILURE = 1,
        EXCEPTION,
        UNKNOWN_ARGUMENT,
        ARGUMENT_MISSING = 10
    };

}

#endif //OSM_LIVE_UPDATES_EXITCODE_H
