#pragma once

#include "redirect.hpp"

enum class TokenType {
  Raw,
  DoubleQuoted,
  SingleQuoted,
  Redirection,
  RedirectionFileDescriptor,
  RedirectionTarget
};

class Parser {
private:
  std::string command;
  std::vector<std::string> arguments;
  std::vector<Redirection> redirections;
  bool noArguments = false;
  bool multiLineCommand = false;

  std::vector<std::string> tokens;
  std::vector<TokenType> tokenTypes;

  void tokenize(const std::string &inputString);
  void parse();

public:
  Parser(const std::string &fullInput, const std::vector<std::string> &history);

  std::string getCommand() const;
  const std::vector<std::string> getArguments() const;
  bool isNoArguments() const;
  bool isMultiLineCommand() const;
  std::vector<std::string> get_null_terminated_args() const;
  const std::vector<std::string> &getTokens() const;
  const std::vector<Redirection> &getRedirections() const;
};
