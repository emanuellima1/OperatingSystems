#include "../include/regular_file.hpp"


RegularFile::RegularFile(int page, std::string name, 
                     time_t creation_time, time_t modification_time, time_t access_time)
    : File(page, name, 'f', creation_time, modification_time,
            access_time) {}

RegularFile::~RegularFile() {}

uint RegularFile::size() {
    return content.length();
}

int RegularFile::wasted_space() {
    // 8: 7 dashes plus one char for type
    int used_space = content.length() + name.length() + 8 + 3 * sizeof(time_t);
    return 4000 - (used_space % 4000);
}
 
