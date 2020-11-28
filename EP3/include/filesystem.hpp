#pragma once

#include <string>

class Filesystem {
public:
    // Creates an object and mounts the filesystem
    Filesystem(std::string filename);

    // Destroy the object and unmounts the filesystem, preserving the file
    ~Filesystem();

    // Prints information about the filesystem
    void df();

private:
    uint n_directories, n_files, free_space;

    // Maximum size of the filesystem in KB
    const uint MAX_SIZE = 100000;
};
