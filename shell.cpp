#include "shell.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

// 按空格拆分字符串
std::vector<std::string> split(const std::string& str)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// 执行带有I/O重定向和管道的命令
void execute_command(const std::vector<std::string>& args)
{
    std::vector<const char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(arg.c_str());
    }
    c_args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) { // 子进程
        // 检查I/O重定向
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == ">") {
                std::ofstream ofs(args[i + 1], std::ofstream::out | std::ofstream::trunc);
                if (!ofs.is_open()) {
                    std::perror("open");
                    std::exit(EXIT_FAILURE);
                }
                std::streambuf* coutbuf = std::cout.rdbuf(); // Save old buffer
                std::cout.rdbuf(ofs.rdbuf());                // Redirect std::cout to file
                c_args[i] = nullptr;
                break;
            } else if (args[i] == "<") {
                std::ifstream ifs(args[i + 1]);
                if (!ifs.is_open()) {
                    std::perror("open");
                    std::exit(EXIT_FAILURE);
                }
                std::streambuf* cinbuf = std::cin.rdbuf(); // Save old buffer
                std::cin.rdbuf(ifs.rdbuf());               // Redirect std::cin to file
                c_args[i] = nullptr;
                break;
            } else if (args[i] == "|") {
                int pipe_fd[2];
                if (pipe(pipe_fd) == -1) {
                    std::perror("pipe");
                    std::exit(EXIT_FAILURE);
                }
                pid_t child_pid = fork();
                if (child_pid == 0) {
                    // 执行管道前的命令
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    execvp(c_args[0], const_cast<char* const*>(c_args.data()));
                    std::perror("execvp 1");
                    std::exit(EXIT_FAILURE);
                } else if (child_pid > 0) {
                    // 执行管道后的命令
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    std::vector<const char*> new_args;
                    for (size_t j = i + 1; j < args.size(); ++j) {
                        new_args.push_back(args[j].c_str());
                    }
                    new_args.push_back(nullptr);
                    execvp(new_args[0], const_cast<char* const*>(new_args.data()));
                    std::perror("execvp 2");
                    std::exit(EXIT_FAILURE);
                } else {
                    std::perror("fork 2");
                    std::exit(EXIT_FAILURE);
                }
            }
        }
        execvp(c_args[0], const_cast<char* const*>(c_args.data()));
        std::perror("execvp 3");
        std::exit(EXIT_FAILURE);
    } else if (pid > 0) { // 父进程
        wait(nullptr);
    } else {
        std::perror("fork 1");
    }
}

// 从脚本文件中读取并执行命令
void execute_script(const std::string& filename)
{
    std::ifstream script_file(filename);
    if (!script_file.is_open()) {
        std::cerr << "错误：无法打开脚本文件 " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(script_file, line)) {
        if (!line.empty()) {
            auto args = split(line);
            execute_command(args);
        }
    }
}
