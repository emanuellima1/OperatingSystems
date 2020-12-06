#pragma once

#include <string>
#include <fstream>
#include <map>
#include <tuple>

#include "file.hpp"

class Directory: public File {
public:
    // Creates a Directory. Called on mkdir.
    Directory(int page, std::string name, 
              uint creation_time, uint modification_time, uint access_time);
    // Destroys a Directory. Called on rmdir.
    ~Directory();

    // Store File object, type and block
    std::map<std::string, std::tuple<File*, char, int>> files;
};
