#pragma once

#include <string>
#include <string_view>
#include <vector>

class Parser {
private:
  std::vector<std::string_view> spaceSplitInput;
  std::string_view command;
  std::vector<std::string_view> arguments;
  bool noArguments = true;
  bool multiLineCommand = false;

  enum class TokenType { Literal, Processed };
  std::vector<std::string> tokens;
  std::vector<TokenType> tokenTypes;

  void tokenize(const std::string &inputString);
  void extractArguments();
  void extractCommand();

public:
  Parser(const std::string &fullInput, const std::vector<std::string> &history);

  std::string_view getCommand() const;
  const std::vector<std::string_view> &getArguments() const;
  bool isNoArguments() const;
  bool isMultiLineCommand() const;
  std::vector<std::string> get_null_terminated_args() const;
};
