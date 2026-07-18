#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <readline/readline.h>

#include "Parser.hpp"

extern char *rl_line_buffer;
extern int rl_point;

// TODO: please somehow generate this at compile time or something from the
// builtins hashmap in bulitins.cpp so you have a single source of truth because
// this is embrrassing
std::vector<std::string> builtins_list{"echo", "type", "exit", "pwd", "cd"};

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

  if (start == 0) {
    return rl_completion_matches(text, command_generator);
  }
  return nullptr;
}

char *command_generator(const char *text, int state) {
  static size_t list_index;
  static int len;

  if (state == 0) {
    list_index = 0;
    len = strlen(text);
  }

  while (list_index < builtins_list.size()) {
    const std::string &name = builtins_list[list_index];
    list_index++;
    if (strncmp(name.c_str(), text, len) == 0) {
      return strdup(name.c_str());
    }
  }
  return nullptr;
}
