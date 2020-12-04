#include "../include/regular_file.hpp"


RegularFile::RegularFile(int page, std::string name, int next_block, 
                     uint creation_time, uint modification_time, uint access_time,
                     Directory *parent)
    : File(page, name, next_block, 'f')
    , parent(parent)
    , creation_time(creation_time)
    , modification_time(modification_time)
    , access_time(access_time) {}

RegularFile::~RegularFile() {}

uint RegularFile::size() {
    return content.length();
}

 
