#pragma once

#include <string>

class File {
public:
    // Creates a File. Called on touch (if the file doesn't exist).
    File(std::string name, uint creation_time, uint size = 0)
        : name { name }
        , creation_time { creation_time } {};

    // Destroys a File. Called on rm.
    ~File();

    // Copy *this into destination
    void cp(File& destination);

    // Shows the content of *this on the screen
    void cat();

    // Changes access_time to now (because the object already exists)
    void touch();

    // Returns the name of *this.
    std::string get_name();

private:
    std::string name;
    uint size, creation_time, modification_time = 0, access_time = 0;
};
