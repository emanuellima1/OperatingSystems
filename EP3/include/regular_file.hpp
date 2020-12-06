#pragma once

#include <string>
#include "directory.hpp"

class RegularFile: public File {
public:
    // Creates a File. Called on touch (if the file doesn't exist).
    RegularFile(int page, std::string name,
              uint creation_time, uint modification_time, uint access_time);

    // Destroys a File. Called on rm.
    ~RegularFile();

    uint size();

    std::string content;
};
