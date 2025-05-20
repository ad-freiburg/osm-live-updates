// Copyright 2025, University of Freiburg
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

#ifndef CHANGEACTIONHELPER_H
#define CHANGEACTIONHELPER_H

namespace olu::osm {

    /**
     * An osm object in a change file can either be inside an <create> <modify> or <delete>
     * XML node. This XML node describes the change action performed on the osm element.
     */
    enum class ChangeAction {
        CREATE, MODIFY, DELETE
    };

}

#endif //CHANGEACTIONHELPER_H
