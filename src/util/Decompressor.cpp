//
// Created by Nicolas von Trott on 31.07.24.
//

#include "util/Decompressor.h"

#include <iostream>
#include <sstream>
#include <string>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

namespace olu::util {
    std::string Decompressor::read(const std::string &path) {
        // Create a stringstream for the compressed input
        std::stringstream compressedStream(path);

        // Create a stringstream for the decompressed output
        std::stringstream decompressedStream;

        // Create a filtering stream buffer for decompression
        boost::iostreams::filtering_streambuf<boost::iostreams::input> filterStream;

        // Add the gzip decompression filter
        filterStream.push(boost::iostreams::gzip_decompressor());

        // Add the compressed input stream to the filter stream
        filterStream.push(compressedStream);

        // Copy the decompressed data to the output stream
        boost::iostreams::copy(filterStream, decompressedStream);

        // Get the decompressed string from the output stream
        return decompressedStream.str();
    }
} //namespace olu::util