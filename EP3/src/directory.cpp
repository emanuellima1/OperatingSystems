#include "../include/directory.hpp"
#include "../include/file.hpp"
#include <iostream>

Directory::Directory(int page, std::string name,
    time_t creation_time, time_t modification_time, time_t access_time)
    : File(page, name, 'd', creation_time, modification_time,
        access_time) {}

Directory::~Directory() { }

int Directory::wasted_space() {
    // 7: 6 dashes plus one char for type
    int used_space = name.length() + 7 + 3 * sizeof(time_t);
    for (auto& [name, t] : files)
        used_space += name.length() + sizeof(int) + 1;

    return 4000 - (used_space % 4000);
}

