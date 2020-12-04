#pragma once

#include <string>
#include <fstream>
#include <map>
#include <tuple>

#include "file.hpp"

class Directory: public File {
public:
    // Creates a Directory. Called on mkdir.
    Directory(int page, std::string name, int next_block, 
              uint creation_time, uint modification_time, uint access_time);
    // Destroys a Directory. Called on rmdir.
    ~Directory();

    // Lists all files and directories inside *this.
    void ls();

    // Search for a file with name filename inside of *this
    void find(std::string filename);

    void add_file(File *file);
    // Store File object, type and block
    std::map<std::string, std::tuple<File*, char, int>> files;
    uint creation_time, modification_time, access_time;
};
