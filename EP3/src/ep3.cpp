#include <array>
#include <iostream>
#include <string>
#include <sstream>

#include "../include/filesystem.hpp"

int main()
{
    std::string command;
    std::array<std::string, 12> possible_commands = { "mount", "cp", "mkdir", "rmdir", "cat", "touch", "rm", "ls", "find", "df", "umount", "sai" };
    std::stringstream token_stream;
    std::string token;

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
        }
        else if (token == "umount")
            delete fs;
        else if (token == "mkdir") {
            token_stream >> token;
            fs->mkdir(token);
        }
        else if (token == "ls") {
            token_stream >> token;
            fs->ls(token);
        }
        else if (token == "touch") {
            token_stream >> token;
            fs->touch(token);
        }
        else if (token == "rm") {
            token_stream >> token;
            fs->rm(token);
        }

        else {
            std::cout << "comando não identificado: " << token << std::endl;
        }

        std::cout << "[ep3]: ";
    }
    std::cout << std::endl;

    return 0;
}
