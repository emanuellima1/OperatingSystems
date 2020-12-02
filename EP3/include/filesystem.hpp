#pragma once

#include <string>
#define MAX_PAGES 25000 // Maximum number of 4.0K pages (total 100MB)
#define PAGE_SIZE 4000
#define HEAD_PAGES (MAX_PAGES/8 + MAX_PAGES)/PAGE_SIZE + 1 // Number of pages used for bitmap and fap

class Filesystem {
public:
    // Creates an object and mounts the filesystem
    Filesystem(std::string filename);

    // Destroy the object and unmounts the filesystem, preserving the file
    ~Filesystem();

    // Prints information about the filesystem
    void df();
    bool bitmap_get(int);
    bool bitmap_get(int, int);
    bool bitmap_set(int, bool); // returns true if set was successful
    bool bitmap_set(int, int, bool);
    int fat_get(int); // returns -1 if set/get was not successful
    int fat_set(int, int);
    int mkdir(std::string);
    int free_page(); // get first free page

private:
    // bitmap and fat tables
    uint bitmap[MAX_PAGES/8];
    int fat[MAX_PAGES];
    std::fstream *fs;
};

