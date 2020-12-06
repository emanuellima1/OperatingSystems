#include "../include/directory.hpp"
#include "../include/file.hpp"
#include <iostream>

Directory::Directory(int page, std::string name, int next_block,
    uint creation_time, uint modification_time, uint access_time)
    : File(page, name, next_block, 'd', creation_time, modification_time,
        access_time) {}

Directory::~Directory() { }

