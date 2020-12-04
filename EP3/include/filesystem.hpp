#pragma once

#include <string>
#include "directory.hpp"
#include "regular_file.hpp"


#define MAX_PAGES 25000 // Maximum number of 4.0K pages (total 100MB)
#define PAGE_SIZE 4000
#define ROOT_PAGE (MAX_PAGES/8 + MAX_PAGES)/PAGE_SIZE + 1

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

private:
    Directory *root;
    std::fstream *fs;
    uint bitmap[MAX_PAGES/8];
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
    void write_dir(Directory*); // write directory to fs
    void write_file(RegularFile*); // write regular file to fs
    File* read_file(int page, Directory *parent); 
};

