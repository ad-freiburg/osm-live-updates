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

#include "util/CacheFile.h"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>

// ____________________________________________________________________________
olu::util::CacheFile::CacheFile(const std::filesystem::path& path)
    : _path(std::filesystem::absolute(path)) {
  reopen();
}

// ____________________________________________________________________________
olu::util::CacheFile::~CacheFile() {
  close();
  remove();
}

// ____________________________________________________________________________
void olu::util::CacheFile::reopen() {
  const int RWRWRW = 0666;
  _fileDescriptor = ::open(_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, RWRWRW);
  if (_fileDescriptor == -1) {
    throw std::filesystem::filesystem_error(
        "Can't open CacheFile", std::filesystem::absolute(_path.c_str()),
        std::make_error_code(std::errc::permission_denied));
  }
}

// ____________________________________________________________________________
void olu::util::CacheFile::write(const std::basic_string<char>& text) const {
    if (_fileDescriptor >= 0) {
        ::write(_fileDescriptor, text.c_str(), text.length());
    } else {
        throw std::filesystem::filesystem_error(
                "Can't write to CacheFile", std::filesystem::absolute(_path.c_str()),
                std::make_error_code(std::errc::permission_denied));
    }
}

// ____________________________________________________________________________
void olu::util::CacheFile::close() {
  if (_fileDescriptor >= 0) {
    ::close(_fileDescriptor);
    _fileDescriptor = -1;
  }
}

// ____________________________________________________________________________
bool olu::util::CacheFile::remove() {
  return std::filesystem::remove(_path);
}

// ____________________________________________________________________________
int olu::util::CacheFile::fileDescriptor() const { return _fileDescriptor; }
