#pragma once
#include <string>
#include <string_view>
#include <vector>

using strviewVector = std::vector<std::string_view>;

size_t skipWhitespace(const std::string &str, size_t startIndex = 0);
strviewVector splitOnSpace(const std::string &str);
std::string expandToken(const std::string &str);
bool isPositiveInteger(const std::string &tokens);
