#pragma once

#include <string>
#include <unordered_set>

extern std::unordered_set<std::string> uniqueCommands;
void gatherCommands();
char **contextAwareCompletion(const char *text, int start, int end);
