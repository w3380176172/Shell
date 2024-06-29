#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

// 按空格拆分字符串
std::vector<std::string> split(const std::string& str);

// 执行带有I/O重定向和管道的命令
void execute_command(const std::vector<std::string>& args);

// 从脚本文件中读取并执行命令
void execute_script(const std::string& filename);

#endif // SHELL_H
