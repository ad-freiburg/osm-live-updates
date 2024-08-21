//
// Created by Nicolas von Trott on 21.08.24.
//

#ifndef OSM_LIVE_UPDATES_OSMCHANGEHANDLEREXCEPTION_H
#define OSM_LIVE_UPDATES_OSMCHANGEHANDLEREXCEPTION_H

#include <stdexcept>
#include  <string>

namespace olu::osm {

    class OsmChangeHandlerException : public std::exception {
    private:
        std::string message;

    public:
        explicit OsmChangeHandlerException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

}

#endif //OSM_LIVE_UPDATES_OSMCHANGEHANDLEREXCEPTION_H
