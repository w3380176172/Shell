#include "shell.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc > 1) {
        std::string filename = argv[1];
        execute_script(filename);
    } else {
        std::string command;
        std::cout << "Welcome to Shell!" << std::endl;
        while (true) {
            std::cout << "shell> ";
            std::getline(std::cin, command);
            if (command == "exit") {
                break;
            }
            if (!command.empty()) {
                auto args = split(command);
                execute_command(args);
            }
        }
    }
    return 0;
}
