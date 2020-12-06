#include "../include/file.hpp"

File::File(int page, std::string name, char type,
           time_t creation_time, time_t modification_time, time_t access_time)
    : page(page)
    , name(name)
    , type (type)
    , creation_time(creation_time)
    , modification_time(modification_time)
    , access_time(access_time)
{
    if (name.length() > MAX_FILENAME)
        name.erase(MAX_FILENAME);
}
