//
// Created by Nicolas von Trott on 26.07.24.
//

#include "osm-live-updates/util/URLHelper.h"
#include "osm-live-updates/config/Constants.h"

#include <stdexcept>

namespace constants = olu::config::constants;

namespace olu::util {

// _________________________________________________________________________________________________
std::string URLHelper::buildUrl(std::vector<std::string> &pathSegments) {
    std::string url;
    for( const auto& segment : pathSegments ) {
        if(&segment == &pathSegments.back() ) {
            url.append(segment);
        } else {
            url.append(segment + "/");
        }
    }

    return url;
}

// _________________________________________________________________________________________________
std::string URLHelper::formatSequenceNumberForUrl(std::string &sequenceNumber) {
    if (sequenceNumber.length() < 1)
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_EMPTY);

    if (sequenceNumber.length() > 9)
        throw std::invalid_argument(constants::EXCEPTION_MSG_SEQUENCE_NUMBER_IS_TOO_LONG);

    while (sequenceNumber.length() < 9)
        sequenceNumber.insert(0, "0");

    std::string part1 = sequenceNumber.substr(0, 3);
    std::string part2 = sequenceNumber.substr(3, 3);
    std::string part3 = sequenceNumber.substr(6, 3);
    return part1 + "/" + part2 + "/" + part3;
}

} // namespace olu::util