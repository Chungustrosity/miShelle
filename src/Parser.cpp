#include "Parser.hpp"
#include "string_utils.hpp"

#include <cctype>
#include <vector>

using std::string;
using std::string_view;
using strVector = std::vector<std::string>;

Parser::Parser(const string &fullInput,
               const std::vector<std::string> &history) {
  tokenize(fullInput);
  extractCommand();
  extractArguments();
}

// Private
void Parser::tokenize(const string &inputString) {
  bool quoted = false;
  std::string token;
  TokenType tokenType;
  size_t l, r;
  l = r = skipWhitespace(inputString);
  while (r < inputString.size()) {
    if (!quoted) {
      while (r < inputString.size() && !std::isspace(inputString[r])) {
        if (inputString[r] == '\'') {
          quoted = true;
          break;
        }
        token += inputString[r];
        r++;
      }
      token = expandToken(token);
      tokenType = TokenType::Processed;

    } else {
      while (inputString[r] != '\'') {
        token += inputString[r];
        r++;
        if (r >= inputString.size()) {
          multiLineCommand = true;
          break;
        }
      }
      quoted = false;
      tokenType = TokenType::Literal;
    }
    tokens.push_back(token);
    tokenTypes.push_back(tokenType);
    token = "";
    l = r = skipWhitespace(inputString, r);
  }
}

void Parser::extractArguments() {
  for (size_t i = 1; i < tokens.size(); i++) {
    arguments.push_back(tokens[i]);
  }
  if (arguments.empty())
    noArguments = true;
}

void Parser::extractCommand() { command = tokens.empty() ? "" : tokens[0]; }

// Public
string Parser::getCommand() const { return command; }
const strVector Parser::getArguments() const { return arguments; }
bool Parser::isNoArguments() const { return noArguments; }
bool Parser::isMultiLineCommand() const { return multiLineCommand; }
const std::vector<std::string> &Parser::getTokens() const { return tokens; }

std::vector<string> Parser::get_null_terminated_args() const {
  std::vector<string> output;
  output.push_back(string(command));
  for (auto &arg : arguments) {
    output.push_back(string(arg));
  }
  return output;
}
