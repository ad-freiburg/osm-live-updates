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

#include "config/Constants.h"
#include "gtest/gtest.h"
#include "util/Decompressor.h"

#include <fstream>

namespace constants = olu::config::constants;
namespace olu::util {

// _________________________________________________________________________________________________
TEST(Decompressor, compressAndDecompress) {
    {
        // Todo: Read path from environment
        std::string result= "/src/tests/data/";

        std::string compressedFilePath = result + "427.osc.gz";
        std::string decompressed = Decompressor::read(compressedFilePath);

        std::string uncompressedFilePath = result + "427.osc";
        std::ifstream t(uncompressedFilePath);
        std::stringstream buffer;
        buffer << t.rdbuf();
        ASSERT_EQ(buffer.str(), decompressed);
    }
}

} // namespace olu::util