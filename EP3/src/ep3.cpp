#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../include/filesystem.hpp"

int main()
{
    std::string command;
    std::stringstream token_stream;
    std::string token;

    std::unique_ptr<Filesystem> fs;

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
            fs = std::make_unique<Filesystem>(token);
        } else if (token == "umount")
            fs.reset();
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
            fs->cp(source, dest);
        } else if (token == "rmdir") {
            token_stream >> token;
            fs->rmdir(token);
        } else if (token == "find") {
            token_stream >> token;
            fs->find(token);
        } else if (token == "df") {
            token_stream >> token;
            fs->df(token);
        } else if (token == "umount") {
            token_stream >> token;
            fs->umount(token);
        } else {
            std::cout << "comando não identificado: " << token << '\n';
        }

        std::cout << "[ep3]: ";
    }

    std::cout << '\n';

    return 0;
}
