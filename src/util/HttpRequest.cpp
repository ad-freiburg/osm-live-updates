// _________________________________________________________________________________________________

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, synchronous for every method on httpbin
//
//------------------------------------------------------------------------------

#include "util/HttpRequest.h"

#include <string>
#include <curl/curl.h>

namespace olu::util {

// _________________________________________________________________________________________________
HttpRequest::HttpRequest(const HttpMethod& method, const std::string& url, bool followLocation) {
    _curl = curl_easy_init();
    _method = method;
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, followLocation);
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
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// _________________________________________________________________________________________________
std::string HttpRequest::performGet() {
    std::string readBuffer;

    if(_curl) {
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &readBuffer);
        _res = curl_easy_perform(_curl);

        return readBuffer;
    }
}

// _________________________________________________________________________________________________
void HttpRequest::performPost() {
    if (_curl) {
        _res = curl_easy_perform(_curl);
    }
}

} // namespace olu::util
