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

#include "util/Decompressor.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <filesystem>

namespace olu::util {

// ____________________________________________________________________________
std::string Decompressor::readGzip(const std::string& path) {
    std::stringstream ss;
    std::ifstream file(path, std::ifstream::in | std::ifstream::binary);
    if (!file) {
        throw std::filesystem::filesystem_error(
                "Can't open file", std::filesystem::absolute(path.c_str()),
                std::make_error_code(std::errc::bad_address));
    }

    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    copy(in, ss);

    return ss.str();
}

std::string Decompressor::readBzip2(const std::string &path) {
    std::stringstream ss;
    std::ifstream file(path, std::ifstream::in | std::ifstream::binary);
    if (!file) {
        throw std::filesystem::filesystem_error(
                "Can't open file", std::filesystem::absolute(path.c_str()),
                std::make_error_code(std::errc::bad_address));
    }

    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::bzip2_decompressor());
    in.push(file);
    copy(in, ss);

    return ss.str();
}

} //namespace olu::util