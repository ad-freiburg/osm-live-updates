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

#include "osm/Osm2ttl.h"
#include "config/Constants.h"
#include "gtest/gtest.h"

namespace olu::osm {
    TEST(Osm2ttl, convertNode) {
        std::string path = "/src/tests/data/";

        std::ifstream ifs(path + "node.osm");
        std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
        std::vector<std::string> elements;
        elements.push_back(nodeElement);

        auto osm2rdf = olu::osm::Osm2ttl();
        auto result = osm2rdf.convert(elements);
        std::string resultAsString;
        for (auto & element : result) {
            resultAsString += element + "\n";
        }

        std::ifstream ifs2(path + "node.ttl");
        std::string groundTruth((std::istreambuf_iterator<char>(ifs2)),
                                (std::istreambuf_iterator<char>()));

        ASSERT_EQ(groundTruth, resultAsString);
    }

    TEST(Osm2ttl, convertWay) {
        std::string path = "/src/tests/data/";

        std::ifstream ifs(path + "wayWithReferences.osm");
        std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
        std::vector<std::string> elements;
        elements.push_back(nodeElement);

        auto osm2rdf = olu::osm::Osm2ttl();
        auto result = osm2rdf.convert(elements);
        std::string resultAsString;
        for (auto & element : result) {
            resultAsString += element + "\n";
        }

        std::ifstream ifs2(path + "way.ttl");
        std::string groundTruth((std::istreambuf_iterator<char>(ifs2)),
                                (std::istreambuf_iterator<char>()));

//        ASSERT_EQ(groundTruth, resultAsString);
    }

    TEST(Osm2ttl, convertRelation) {
        std::string path = "/src/tests/data/";

        std::ifstream ifs(path + "relation.osm");
        std::string nodeElement((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
        std::vector<std::string> elements;
        elements.push_back(nodeElement);

        auto osm2rdf = olu::osm::Osm2ttl();
        auto result = osm2rdf.convert(elements);
        std::string resultAsString;
        for (auto & element : result) {
            resultAsString += element + "\n";
        }

        std::ifstream ifs2(path + "relation.ttl");
        std::string groundTruth((std::istreambuf_iterator<char>(ifs2)),
                                (std::istreambuf_iterator<char>()));

//        ASSERT_EQ(std::sort(groundTruth), std::sort(resultAsString));
    }
} // namespace olu::osm
