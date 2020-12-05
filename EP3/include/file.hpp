#pragma once

#include <string>

#define MAX_FILENAME 255

class File {
public:
    // Creates a File.
    File(int page, std::string name, int next_block, char type,
         uint creation_time, uint modification_time, uint access_time);
    virtual ~File() {};
    int page;
    std::string name;
    int next_block;
    char type;
    uint creation_time, modification_time, access_time;
};
