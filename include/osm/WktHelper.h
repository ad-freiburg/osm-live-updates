//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_WKTHELPER_H
#define OSM_LIVE_UPDATES_WKTHELPER_H

#include <string>

namespace olu::osm {
    class WktHelper {
    public:
        // Creates a dummy node with an id and a location which is extracted from an WKT point.
        static std::string createDummyNodeFromPoint(const std::string& nodeId,
                                                    const std::string& pointAsWkt);
    };

    class WktHelperException : public std::exception {
    private:
        std::string message;

    public:
        explicit WktHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };
}
#endif //OSM_LIVE_UPDATES_WKTHELPER_H
