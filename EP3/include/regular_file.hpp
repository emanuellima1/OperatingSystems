#pragma once

#include <string>
#include "directory.hpp"

class RegularFile: public File {
public:
    // Creates a File. Called on touch (if the file doesn't exist).
    RegularFile(int page, std::string name,
              time_t creation_time, time_t modification_time, time_t access_time);

    // Destroys a File. Called on rm.
    ~RegularFile();

    uint size();

    std::string content;

    int wasted_space();
};
