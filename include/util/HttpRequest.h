// Copyright 2024, University of Freiburg
// Authors: Nicolas von Trott <nicolasvontrott@gmail.com>.

// This file is part of osm-live-updates.
//
// osm-live-updates is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm-live-updates is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm-live-updates.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM_LIVE_UPDATES_HTTPREQUEST_H
#define OSM_LIVE_UPDATES_HTTPREQUEST_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include "HttpMethod.h"

namespace olu::util {

    class HttpRequest {
    public:
        explicit HttpRequest(
                const HttpMethod& method,
                const std::string& url);
        ~HttpRequest();
        void addHeader(const std::string& key, const std::string& value);
        void addBody(std::string body);
        std::string perform();
    private:
        CURL *_curl;
        HttpMethod _method;
        CURLcode _res;
        std::string _data;
        std::string _body;

        struct curl_slist *_chunk = nullptr;
    };

    class HttpRequestException : public std::exception {
    private:
        std::string message;

    public:
        explicit HttpRequestException(const char* msg) : message(msg) { }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

} // namespace olu::util

#endif //OSM_LIVE_UPDATES_HTTPREQUEST_H
