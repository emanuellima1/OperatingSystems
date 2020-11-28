#pragma once

#include <string>

class Directory {
public:
    // Creates a Directory. Called on mkdir.
    Directory(std::string name, uint creation_time)
        : name { name }
        , creation_time { creation_time } {};

    // Destroys a Directory. Called on rmdir.
    ~Directory();

    // Lists all files and directories inside *this.
    void ls();

    // Search for a file with name filename inside of *this
    void find(std::string filename);

private:
    std::string name;
    uint creation_time, modification_time = 0, access_time = 0;
};
