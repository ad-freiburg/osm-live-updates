//
// Created by Nicolas von Trott on 18.06.25.
//

#ifndef TRIPLE_H
#define TRIPLE_H

#include <string>

namespace olu::ttl {
    struct Triple {
        std::string subject;
        std::string predicate;
        std::string object;
    };

    inline std::string to_string(const Triple& triple) {
        return triple.subject + " " + triple.predicate + " " + triple.object;
    }
}

#endif //TRIPLE_H
