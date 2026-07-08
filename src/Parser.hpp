#pragma once

#include <string>
#include <vector>

enum class TokenType { Raw, DoubleQuoted, SingleQuoted };

class Parser {
private:
  std::string command;
  std::vector<std::string> arguments;
  bool noArguments = false;
  bool multiLineCommand = false;

  std::vector<std::string> tokens;
  std::vector<TokenType> tokenTypes;

  void tokenize(const std::string &inputString);
  void extractArguments();
  void extractCommand();

public:
  Parser(const std::string &fullInput, const std::vector<std::string> &history);

  std::string getCommand() const;
  const std::vector<std::string> getArguments() const;
  bool isNoArguments() const;
  bool isMultiLineCommand() const;
  std::vector<std::string> get_null_terminated_args() const;
  const std::vector<std::string> &getTokens() const;
};
