#include "Parser.hpp"
#include "string_utils.hpp"

#include <cctype>

using std::string;
using std::string_view;
using strviewVector = std::vector<std::string_view>;

Parser::Parser(const string &fullInput,
               const std::vector<std::string> &history) {
  spaceSplitInput = splitOnSpace(fullInput);
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
  while (l < inputString.size()) {
    if (!quoted) {
      while (r < inputString.size() && !std::isspace(inputString[r])) {
        if (inputString[r] == '\'') {
          quoted = true;
          break;
        }
        token += inputString[r];
      }
      token = expandToken(token);
      tokenType = TokenType::Processed;

    } else {
      while (inputString[r] != '\'') {
        token += inputString[r];
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
  for (size_t i = 1; i < spaceSplitInput.size(); i++) {
    arguments.push_back(spaceSplitInput[i]);
  }
  if (!arguments.empty())
    noArguments = false;
}

void Parser::extractCommand() {
  command = spaceSplitInput.empty() ? "" : spaceSplitInput[0];
}

// Public
string_view Parser::getCommand() const { return command; }
const strviewVector &Parser::getArguments() const { return arguments; }
bool Parser::isNoArguments() const { return noArguments; }
bool Parser::isMultiLineCommand() const { return multiLineCommand; }

std::vector<string> Parser::get_null_terminated_args() const {
  std::vector<string> output;
  output.push_back(string(command));
  for (auto &arg : arguments) {
    output.push_back(string(arg));
  }
  return output;
}
