//
// Created by Nicolas von Trott on 26.07.24.
//

#ifndef OSM_LIVE_UPDATES_URLHELPER_H
#define OSM_LIVE_UPDATES_URLHELPER_H

#include <string>
#include <vector>

namespace olu::util {

class URLHelper {
public:
    // Builds an url from a list of strings by concatenating them with a '/'
    static std::string buildUrl(std::vector<std::string> &pathSegments);

    // Formats a sequence number for use in an url
    // For example: The sequence number 6177383 would be returned as 006/177/383
    // @throw 'std::invalid_argument' if sequence number is empty or too long
    static std::string formatSequenceNumberForUrl(std::string &sequenceNumber);

    static std::string encodeForUrlQuery(const std::string& value);
};

} // namespace olu::util

#endif //OSM_LIVE_UPDATES_URLHELPER_H
