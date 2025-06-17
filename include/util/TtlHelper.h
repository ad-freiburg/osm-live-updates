//
// Created by Nicolas von Trott on 03.12.24.
//

#ifndef TTLHELPER_H
#define TTLHELPER_H

#include <string>

#include "util/Types.h"
#include "osm/OsmObjectType.h"

namespace olu::util {

    class TtlHelper {
    public:
        /**
         * Parses a triple string into its components.
         * This function is not doing any validation because it is intended to be used with the
         * output from osm2rdf, which is assumed to be in the correct format:
         * "subject predicate object ."
         *
         * @param tripleString The triple string to parse, in the format:
         * "subject predicate object ."
         * @return A tuple containing strings with the subject, predicate, and object of the triple
         */
        static triple_t parseTriple(const std::string& tripleString);

        /**
         * Returns the triple as a string in the format:
         * "subject predicate object ."
         */
        static std::string getTripleString(const triple_t& triple) {
            return std::get<0>(triple) + " " + std::get<1>(triple) + " " + std::get<2>(triple);
        }

        /**
         * Parses the id from a prefixed name like "osmnode:1" or "osm2rdfgeom:osm_way_centroid_1".
         * The prefixed has to end with the id number. There is again no validation of the input,
         * so make sure it is in the correct format.
         *
         * @param prefixedName The prefixed name to extract the id from.
         * @return The extracted id as an id_t type.
         */
        static id_t parseId(const std::string& prefixedName);

        /**
         * Checks if the given subject is in the relevant namespace for the given osm object type:
         * "osmnode: ..." for nodes, "osmway: ..." for ways and "osmrel: ..." for relations.
         *
         * @param subject The subject string to check.
         * @param osmObject The osm object type to check the namespace for.
         * @return True if the subject is in the relevant namespace, false otherwise.
         */
        static bool
        isInNamespaceForOsmObject(const std::string& subject, const osm::OsmObjectType & osmObject);

        /**
         * Checks if the given predicate describes a tag or metadata of the given osm object.
         *
         * @param predicate The predicate string to check.
         * @param osmObject The osm object type to check the predicate for.
         * @return True if the predicate describes a tag or metadata, false otherwise.
         */
        static bool
        isMetadataOrTagPredicate(const std::string& predicate,
                                 const osm::OsmObjectType & osmObject);

        /**
         * Checks if the given predicate describes the geometry of the given osm object.
         *
         * @param predicate The predicate string to check.
         * @param osmObject The osm object type to check the predicate for.
         * @return True if the predicate describes the geometry, false otherwise.
         */
        static bool
        isGeometryPredicate(const std::string& predicate, const osm::OsmObjectType & osmObject);

        /**
         * Checks if a predicate links to an object,
         * which has a triple that is relevant to an osm object. For example, members of ways and
         * relations have triples on their own, which are relevant for the way or relation:
         * "osmrel:11892035 osmrel:member _:6_168 ."
         * "_:6_168 osm2rdfmember:id osmway:1058514204 ."
         * The object "_6_168" is a blank node, which links to the triple beneath.
         *
         * @param predicate The predicate string to check.
         * @param osmObject The osm object type to check the predicate for.
         * @return True if the predicate links to an object with a relevant triple, false otherwise.
         */
        static bool
        hasRelevantObject(const std::string& predicate, const osm::OsmObjectType & osmObject);

    };

    /**
    * Exception that can appear inside the `TtlHelper` class.
    */
    class TtlHelperException final : public std::exception {
        std::string message;

    public:
        explicit TtlHelperException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::util


#endif //TTLHELPER_H
