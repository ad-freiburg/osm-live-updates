//
// Created by Nicolas von Trott on 03.12.24.
//

#ifndef TTLHELPER_H
#define TTLHELPER_H

#include "Types.h"

#include <string>

namespace olu::util {

    class TtlHelper {
    public:
        static Triple getTriple(const std::string& tripleString);
        static id_t getIdFromSubject(const std::string& subject, const std::string &osmTag);
        static bool isRelevantNamespace(const std::string& subject, const std::string &osmTag);
        static bool hasRelevantObject(const std::string& predicate, const std::string &osmTag);
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
