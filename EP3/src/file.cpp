#include "../include/file.hpp"

File::File(int page, std::string name, int next_block, char type)
    : page(page)
    , name(name)
    , next_block(next_block)
    , type (type) {}