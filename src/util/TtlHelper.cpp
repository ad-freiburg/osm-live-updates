//
// Created by Nicolas von Trott on 03.12.24.
//

#include "util/TtlHelper.h"
#include "config/Constants.h"

#include <boost/regex.hpp>

namespace cnst = olu::config::constants;
namespace olu::util {

    triple_t TtlHelper::getTriple(const std::string& triple) {
        const boost::regex regex(R"((\S+)\s(\S+)\s(.*)\s\.)");
        if (boost::smatch match; boost::regex_search(triple, match, regex)) {
            return std::make_tuple(match[1], match[2], match[3]);
        }

        const std::string msg = "Cant split triple: " + triple;
        throw TtlHelperException(msg.c_str());
    }

    bool TtlHelper::isRelevantNamespace(const std::string& subject, const std::string &osmTag) {
        if (osmTag == cnst::XML_TAG_NODE) {
            return subject.starts_with(cnst::NAMESPACE_OSM_NODE);
        }

        if (osmTag == cnst::XML_TAG_WAY) {
            return subject.starts_with(cnst::NAMESPACE_OSM_WAY);
        }

        if (osmTag == cnst::XML_TAG_REL) {
            return subject.starts_with(cnst::NAMESPACE_OSM_REL);
        }

        const std::string msg = "Cant interpret subject: " + subject;
        throw TtlHelperException(msg.c_str());
    }

    bool TtlHelper::hasRelevantObject(const std::string& predicate, const std::string &osmTag) {
        if (osmTag == cnst::XML_TAG_NODE) {
            return predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
        }

        if (osmTag == cnst::XML_TAG_WAY) {
            return predicate == cnst::PREFIXED_WAY_MEMBER ||
                   predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
        }

        if (osmTag == cnst::XML_TAG_REL) {
            return predicate == cnst::PREFIXED_REL_MEMBER ||
                   predicate == cnst::PREFIXED_GEO_HAS_CENTROID ||
                   predicate == cnst::PREFIXED_GEO_HAS_GEOMETRY;
        }

        const std::string msg = "Cant interpret predicate: " + predicate;
        throw TtlHelperException(msg.c_str());
    }

    id_t TtlHelper::getIdFromSubject(const std::string& subject, const std::string& osmTag) {
        std::string regexString;
        if (osmTag == cnst::XML_TAG_NODE) {
            regexString = R"((?:osmnode:|osm_node_|osm_node_centroid_)(\d+))";
        } else if (osmTag == cnst::XML_TAG_WAY) {
            regexString = R"((?:osmway:|osm_wayarea_)(\d+))";
        } else if (osmTag == cnst::XML_TAG_REL) {
            regexString = R"((?:osmrel:|osm_relarea_)(\d+))";
        } else {
            const std::string msg = "Unknown element tag: " + osmTag;
            throw TtlHelperException(msg.c_str());
        }

        const boost::regex integerRegex(regexString);
        if (boost::smatch match; boost::regex_search(subject, match, integerRegex)) {
            return stoll(match[1]);
        }

        const std::string msg = "Cant get id for " + osmTag + " from triple: " + subject;
        throw TtlHelperException(msg.c_str());
    }
}