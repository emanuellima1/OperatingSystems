#pragma once

#include <string>
#include "directory.hpp"

class RegularFile: public File {
public:
    // Creates a File. Called on touch (if the file doesn't exist).
    RegularFile(int page, std::string name, int next_block, 
              uint creation_time, uint modification_time, uint access_time,
              Directory *parent);

    // Destroys a File. Called on rm.
    ~RegularFile();

    // Copy *this into destination
    void cp(RegularFile& destination);

    // Shows the content of *this on the screen
    void cat();

    // Changes access_time to now (because the object already exists)
    void touch();

    // Returns the name of *this.
    std::string get_name();

private:
    std::string name;
    uint size, creation_time, modification_time, access_time;
};
