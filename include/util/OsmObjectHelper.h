//
// Created by Nicolas von Trott on 28.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
#define OSM_LIVE_UPDATES_OSMOBJECTHELPER_H

#include <string>
#include <set>
#include <vector>

namespace olu::osm {
    class OsmObjectHelper {
    public:
        /**
         * Returns a node with an id and a location which is extracted from an WKT point.
         *
         * @example For nodeId: `1` and pointAsWkt: `POINT(13.5690032 42.7957187)` the function
         * would return: `<node id="1" lat="42.7957187" lon="13.5690032"/>`
         *
         * @param nodeId The node id that should be used for the node
         * @param pointAsWkt The point in WKT format from which the location of the dummy node
         * should be extracted
         */
        static std::string
        createNodeFromPoint(const long long &nodeId, const std::string& pointAsWkt);

        /**
         * Returns a way with an id and node references.
         *
         * @example For wayId: `1` and nodeRefs: `{1,2,3}` the function
         * would return: `<way id="1"><nd ref="1"/><nd ref="2"/><nd ref="3"/></way>`
         *
         * @param wayId The way id that should be used for the way
         * @param nodeRefs The ids of the nodes that are referenced by the way
         */
        static std::string
        createWayFromReferences(long long wayId, const std::set<long long> &nodeRefs);

        /**
         * Returns a relation with an id and members.
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
         *
         * @param relationId The relation id that should be used for the relation
         * @param members A pair containing the uri and role of each member of the relation
         */
        static std::string
        createRelationFromReferences(long long relationId,
                                     const std::vector<std::pair<std::string, std::string>> &members);
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
#endif //OSM_LIVE_UPDATES_OSMOBJECTHELPER_H
