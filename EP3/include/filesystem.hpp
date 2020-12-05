#pragma once

#include <string>
#include <climits>
#include "directory.hpp"
#include "regular_file.hpp"
#include <string>

#define MAX_PAGES 25000 // Maximum number of 4.0K pages (total 100MB)
#define PAGE_SIZE 4000
// The following use the form (n + d - 1)/d to get the ceiling of n/d
#define BITMAP_SIZE (MAX_PAGES + CHAR_BIT - 1)/CHAR_BIT
#define ROOT_PAGE ((BITMAP_SIZE + MAX_PAGES * sizeof(int)) + PAGE_SIZE - 1)/PAGE_SIZE

class Filesystem {
public:
    // Creates an object and mounts the filesystem
    Filesystem(std::string filename);

    // Destroy the object and unmounts the filesystem, preserving the file
    ~Filesystem();

    // Prints information about the filesystem
    void df();
    void mkdir(std::string); // write regular file to fs
    void ls(std::string path);
    void touch(std::string path);
    void rm(std::string path);

private:
    Directory *root;
    std::fstream *fs;
    char bitmap[BITMAP_SIZE];
    int fat[MAX_PAGES];

    bool bitmap_get(int);
    bool bitmap_get(int, int);
    bool bitmap_set(int, bool); // returns true if set was successful
    bool bitmap_set(int, int, bool);
    int fat_get(int); // returns -1 if set/get was not successful
    int fat_set(int, int);

    int free_page(); // get first free page
    File* get_file(std::string path);
    std::string filename(std::string path);
    std::string dirname(std::string path);
    void write_file(File*); // write file to fs
    File* read_file(int page);
    void delete_dir(Directory *d);
};
