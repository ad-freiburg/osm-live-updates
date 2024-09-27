//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_WKTHELPER_H
#define OSM_LIVE_UPDATES_WKTHELPER_H

#include <string>

namespace olu::osm {
    class WktHelper {
    public:
        /**
         * Creates a dummy node with an id and a location which is extracted from an WKT point.
         *
         * Example: Foe node id: `1` and pointAsWkt: `POINT(13.5690032 42.7957187)` the function
         * would return: `<node id="1" lat="42.7957187" lon="13.5690032"/>`
         *
         * @param nodeId The node id that should be used for the dummy node
         * @param pointAsWkt The point in WKT format from which the location of the dummy node
         * should be extracted
         * @return A dummy node containing the given node id and location
         */
        static std::string
        createDummyNodeFromPoint(const int &nodeId, const std::string& pointAsWkt);
    };

    /**
     * Exception that can appear inside the `WktHelper` class.
     */
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
