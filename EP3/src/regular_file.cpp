#include "../include/regular_file.hpp"


RegularFile::RegularFile(int page, std::string name, int next_block, 
                     uint creation_time, uint modification_time, uint access_time)
    : File(page, name, next_block, 'f', creation_time, modification_time,
            access_time) {}

RegularFile::~RegularFile() {}

uint RegularFile::size() {
    return content.length();
}

 
