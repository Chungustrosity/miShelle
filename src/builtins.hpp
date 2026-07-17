#pragma once

#include "Parser.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using string = std::string;

enum class CommandType { builtin, external, notACommand };
bool isBuiltin(const Parser &parsedInput);
int executeBuiltin(const Parser &parsedInput);

int echo_builtin(const Parser &parsedInput);

#if !defined(_WIN32)
#include <unistd.h>
#endif

#if defined(_WIN32)
const char DELIM = ';';
#else
const char DELIM = ':';
#endif

bool canBeExecuted(const fs::path &filePath);
fs::path findPathInDirectory(const string &dirPath, const string &commandName);

fs::path findInPATH(const string &command);

int type_builtin(const Parser &parsedInput);

CommandType findCommandType(const string &command);
int exit_builtin(const Parser &parsedInput);

// TODO: refactor so you don't need to check if the command is
// CommandType::external in the main loop just to apply some of the same logic
// again here to get the filePath
int execute_external(const Parser &parsedInput);

int pwd_builtin(const Parser &parsedInput);
int cd_builtin(const Parser &parsedInput);
