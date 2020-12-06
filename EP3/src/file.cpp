#include "../include/file.hpp"

File::File(int page, std::string name, char type,
           uint creation_time, uint modification_time, uint access_time)
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
