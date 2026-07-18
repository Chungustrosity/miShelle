#include "reading.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#include <readline/readline.h>

#include "Parser.hpp"
#include "builtins.hpp"

static std::unordered_set<std::string> uniqueCommands;
namespace fs = std::filesystem;
void appendBuiltinsToSet(std::unordered_set<std::string> &uniqueCommands) {
  for (const auto &[commandName, commandFunction] : builtins) {
    uniqueCommands.insert(commandName);
  }
}
void pushViableCommandsToSet(const std::string &dirPath,
                             std::unordered_set<std::string> &commandSet) {
  if (dirPath.empty() || !fs::exists(dirPath) || !fs::is_directory(dirPath))
    return;
  for (const auto &dirEntry : fs::directory_iterator(dirPath)) {
    if (fs::is_regular_file(dirEntry.status())) {
      if (access(dirEntry.path().c_str(), X_OK) == 0) {
        commandSet.insert(dirEntry.path().filename().string());
      }
    }
  }
}
void gatherCommands() {
  const char DELIM = ':';
  const char *rawPATH = std::getenv("PATH");
  std::string PATH = std::string(rawPATH ? rawPATH : "") + DELIM;
  size_t l = 0, r = 0;

  if (PATH == std::string(1, DELIM)) {
    std::cerr << "PATH Environment Variable wasn't found\n";
    // return " ";
  }

  std::string dir;
  for (int i = 0; i < PATH.size(); i++) {
    if (PATH[i] == DELIM) {
      try {
        pushViableCommandsToSet(dir, uniqueCommands);
      } catch (const fs::filesystem_error &e) {
        std::cerr << "Directory " << dir << " traversal error: " << e.what()
                  << "\n";
      }
      dir = "";
    } else {
      dir += PATH[i];
    }
  }
  appendBuiltinsToSet(uniqueCommands);
}

extern char *rl_line_buffer;
extern int rl_point;

char *command_generator(const char *text, int state);

char **contextAwareCompletion(const char *text, int start, int end) {
  std::string inputSoFar(rl_line_buffer, rl_point);
  std::string paddedInput = " " + inputSoFar + "dummy" + " ";
  Parser parser(paddedInput);
  const auto &tokens = parser.getTokens();
  const auto &tokenTypes = parser.getTokenTypes();
  TokenType currentTokenType = tokenTypes.back();
  switch (currentTokenType) {
  case TokenType::Command:
    return rl_completion_matches(text, command_generator);
  case TokenType::Raw:
  case TokenType::DoubleQuoted:
  case TokenType::SingleQuoted:
  case TokenType::RedirectionTarget:

  // impossible
  case TokenType::RedirectionFileDescriptor:
  case TokenType::Redirection:
  default:
    return nullptr;
  }
}

char *command_generator(const char *text, int state) {
  static std::unordered_set<std::string>::const_iterator it;
  static int len;

  if (state == 0) {
    it = uniqueCommands.begin();
    len = strlen(text);
  }

  while (it != uniqueCommands.end()) {
    const std::string &command = *it;
    it++;
    if (strncmp(command.c_str(), text, len) == 0)
      return strdup(command.c_str());
  }
  return nullptr;
}
