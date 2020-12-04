#pragma once

#include <string>

class File {
public:
    // Creates a File.
    File(int, std::string, int, char);
    virtual ~File() {};
    int page;
    std::string name;
    int next_block;
    char type;
};
