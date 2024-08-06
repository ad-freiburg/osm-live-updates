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

#ifndef OSM_LIVE_UPDATES_CACHEFILE_H_
#define OSM_LIVE_UPDATES_CACHEFILE_H_

#include <filesystem>

namespace olu::util {

class CacheFile {
 public:
  // Creates CacheFile at given path.
  explicit CacheFile(const std::filesystem::path& path);
  // Closes and removes files.
  ~CacheFile();
  // Opens file.
  void reopen();
  // Write to file if open.
  void write(const std::basic_string<char>& text) const;
  // Closes file if open.
  void close();
  // Removes file.
  bool remove();
  // Returns file descriptor for use in libosmium.
  [[nodiscard]] int fileDescriptor() const;

 protected:
  std::filesystem::path _path;
  int _fileDescriptor = -1;
};

}  // namespace olu::util

#endif  // OSM_LIVE_UPDATES_CACHEFILE_H_
