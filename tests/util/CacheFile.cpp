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

#include "config/Config.h"
#include "gtest/gtest.h"
#include "util/CacheFile.h"

#include <fstream>

namespace olu::util {

// ____________________________________________________________________________
    TEST(CacheFile, write) {
        olu::config::Config config;
        std::filesystem::path location{config.getTempPath(
                "UTIL_CacheFile_write", "constructor-output")};

        olu::util::CacheFile cf(location);

        std::string testString = "Test";
        cf.write(testString);
        cf.close();

        std::ifstream ifs(location);
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ) );

        ASSERT_EQ(testString, content);
    }

// ____________________________________________________________________________
    TEST(CacheFile, constructorAndAutoRemove) {
        olu::config::Config config;
        std::filesystem::path location{config.getTempPath(
                "UTIL_CacheFile_constructorAndAutoRemove", "constructor-output")};

        ASSERT_FALSE(std::filesystem::exists(location));
        {
            olu::util::CacheFile cf(location);
            ASSERT_NE(-1, cf.fileDescriptor());
            ASSERT_TRUE(std::filesystem::exists(location));
        }
        ASSERT_FALSE(std::filesystem::exists(location));
    }

// ____________________________________________________________________________
    TEST(CacheFile, close) {
        olu::config::Config config;
        std::filesystem::path location{
                config.getTempPath("UTIL_CacheFile_close", "constructor-output")};

        ASSERT_FALSE(std::filesystem::exists(location));
        {
            olu::util::CacheFile cf(location);
            ASSERT_NE(-1, cf.fileDescriptor());
            ASSERT_TRUE(std::filesystem::exists(location));
            cf.close();
            ASSERT_EQ(-1, cf.fileDescriptor());
            cf.close();
            ASSERT_EQ(-1, cf.fileDescriptor());
        }
        ASSERT_FALSE(std::filesystem::exists(location));
    }

// ____________________________________________________________________________
    TEST(CacheFile, remove) {
        olu::config::Config config;
        std::filesystem::path location{
                config.getTempPath("UTIL_CacheFile_remove", "constructor-output")};

        ASSERT_FALSE(std::filesystem::exists(location));
        {
            olu::util::CacheFile cf(location);
            ASSERT_NE(-1, cf.fileDescriptor());
            ASSERT_TRUE(std::filesystem::exists(location));
            cf.close();
            cf.remove();
            ASSERT_FALSE(std::filesystem::exists(location));
        }
        ASSERT_FALSE(std::filesystem::exists(location));
    }

}