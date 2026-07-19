#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

class ShellState {
public:
  void init();
  std::unordered_map<std::string, std::filesystem::path> completionSpecs;
};

extern ShellState global_state;
