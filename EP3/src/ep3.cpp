#include <array>
#include <iostream>
#include <string>

#include "../include/directory.hpp"
#include "../include/file.hpp"

int main()
{
    std::string command;
    std::array<std::string, 12> possible_commands = { "mount", "cp", "mkdir", "rmdir", "cat", "touch", "rm", "ls", "find", "df", "umount", "sai" };

    std::cout << "[ep3]: ";
    while (std::getline(std::cin, command)) {
        if (command == "sai") {
            return 0;
        }
        std::cout << command << '\n';
        std::cout << "[ep3]: ";
    }

    return 0;
}