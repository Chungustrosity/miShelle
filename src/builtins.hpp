#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

#include "Parser.hpp"
#include "state.hpp"

#if defined(_WIN32)
const char DELIM = ';';
#else
#include <unistd.h>
const char DELIM = ':';
#endif

namespace fs = std::filesystem;
using string = std::string;

extern const std::unordered_map<
    string, std::function<int(const Statement &, ShellState &)>>
    builtins;
enum class CommandType { builtin, external, notACommand };
CommandType findCommandType(const string &command);
bool isBuiltin(const std::string &commandName);
// int executeBuiltinCommandStatement(const Parser &parsedInput,
//                                    ShellState &state);
int executeBuiltinCommandStatement(const Statement &statement,
                                   ShellState &state);
// int execute_external(const Parser &parsedInput, ShellState &state);
int execute_external(const Statement &statement, ShellState &state);

bool canBeExecuted(const fs::path &filePath);
fs::path findPathInDirectory(const string &dirPath, const string &commandName);
fs::path findInPATH(const string &command);

int type_builtin(const Statement &statement, ShellState &state);
int jobs_builtin(const Statement &statement, ShellState &state);
int echo_builtin(const Statement &statement, ShellState &state);
int exit_builtin(const Statement &statement, ShellState &state);
int pwd_builtin(const Statement &statement, ShellState &state);
int cd_builtin(const Statement &statement, ShellState &state);
int complete_builtin(const Statement &statement, ShellState &state);
