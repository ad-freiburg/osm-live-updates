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

#include "util/HttpRequest.h"

#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace olu::util {

// _________________________________________________________________________________________________
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    auto& mem = *static_cast<std::string*>(userp);
    mem.append(static_cast<char*>(contents), realsize);
    memset(contents, 0, realsize);
    return realsize;
}

// _________________________________________________________________________________________________
void setup_curl(CURL* curl_handle, std::string& data, const std::string& url)
{
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

//    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
}

// _________________________________________________________________________________________________
HttpRequest::HttpRequest(const HttpMethod& method, const std::string& url) {
    _curl = curl_easy_init();
    _method = method;
    setup_curl(_curl, _data, url);
}

// _________________________________________________________________________________________________
HttpRequest::~HttpRequest() {
    curl_easy_cleanup(_curl);
}

// _________________________________________________________________________________________________
void HttpRequest::addHeader(const std::string& key, const std::string& value) {
    std::string header = key + ": " + value;
    curl_easy_setopt(_curl, CURLOPT_HEADER, header.c_str());
}

// _________________________________________________________________________________________________
void HttpRequest::addBody(const std::string& body) {
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, body.c_str());
}

// _________________________________________________________________________________________________
std::string HttpRequest::perform() {
    std::string response;
    if (_method == HttpMethod::GET) {
        response =  performGet();
    } else {
        performPost();
    }

    if (_res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(_res));
    }

    return response;
}

// _________________________________________________________________________________________________
std::string HttpRequest::performGet() {
    if(_curl) {
        _res = curl_easy_perform(_curl);
        return _data;
    }
}

// _________________________________________________________________________________________________
void HttpRequest::performPost() {
    if (_curl) {
        _res = curl_easy_perform(_curl);
    }
}

std::vector<std::string> HttpRequest::multiPerform(const std::vector<std::string> &urls) {
    CURLM* multi_handle;
    int still_running;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    multi_handle = curl_multi_init();

    std::vector<CURL*> curl_handles;
    std::vector<std::string> dataVec(urls.size());

    for (size_t i = 0; i < urls.size(); ++i)
    {
        CURL* curl_handle = curl_easy_init();
        setup_curl(curl_handle, dataVec[i], urls[i]);
        curl_handles.push_back(curl_handle);
        curl_multi_add_handle(multi_handle, curl_handle);
    }

    curl_multi_perform(multi_handle, &still_running);

    while (still_running)
    {
        struct timeval timeout;
        int rc;

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd;

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

        switch (rc)
        {
            case -1:
                break;
            case 0:
            default:
                curl_multi_perform(multi_handle, &still_running);
                break;
        }
    }

    for (CURL* handle : curl_handles)
    {
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
    }
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();

    return dataVec;
}

} // namespace olu::util
