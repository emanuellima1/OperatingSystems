#pragma once

#include <string>

#define MAX_FILENAME 255

class File {
public:
    // Creates a File.
    File(int page, std::string name, char type,
         time_t creation_time, time_t modification_time, time_t access_time);
    virtual ~File() {};
    int page;
    std::string name;
    char type;
    time_t creation_time, modification_time, access_time;

    virtual int wasted_space() = 0;
};
