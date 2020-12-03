#include "../include/file.hpp"

File::File(int page, std::string name, int next_block)
    : page(page)
    , name(name)
    , next_block(next_block) {}
