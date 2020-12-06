#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../include/filesystem.hpp"

int main()
{
    std::string command;
    std::stringstream token_stream;
    std::string token, token2;

    Filesystem *fs;

    std::cout << "[ep3]: ";
    while (std::getline(std::cin, command)) {
        token_stream.clear();
        token.clear();
        token_stream.str(command);
        token_stream >> token;

        if (token == "sai")
            return 0;
        else if (token == "mount") {
            token_stream >> token;
            fs = new Filesystem(token);
        } else if (token == "umount")
          delete fs;
        else if (token == "mkdir") {
            token_stream >> token;
            fs->mkdir(token);
        } else if (token == "ls") {
            token_stream >> token;
            fs->ls(token);
        } else if (token == "touch") {
            token_stream >> token;
            fs->touch(token);
        } else if (token == "rm") {
            token_stream >> token;
            fs->rm(token);
        } else if (token == "cat") {
            token_stream >> token;
            fs->cat(token);
        } else if (token == "cp") {
            token_stream >> token;
            token_stream >> token2;
            fs->cp(token, token2);
        } else if (token == "rmdir") {
            token_stream >> token;
            fs->rmdir(token);
        } else if (token == "find") {
            token_stream >> token;
            token_stream >> token2;
            fs->find(token, token2);
        } else if (token == "df") {
            fs->df();
        } else {
            std::cout << "comando não identificado: " << token << '\n';
        }

        std::cout << "[ep3]: ";
    }

    std::cout << '\n';

    return 0;
}
