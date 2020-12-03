#pragma once

#include <string>

class File {
public:
    // Creates a File.
    File(int, std::string, int);
    virtual ~File() {};
    int page;
    std::string name;
    int next_block;
};
