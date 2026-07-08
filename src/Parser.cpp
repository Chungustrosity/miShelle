#include "Parser.hpp"
#include "string_utils.hpp"

#include <cctype>
#include <cstddef>
#include <functional>
#include <print>
#include <string>
#include <unordered_set>
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
// input string is padded in the main loop so boundry checks aren't necessary
// for a single index more/less than r
void concatWithPrevious(const size_t i, string &token,
                        std::vector<string> &tokens,
                        std::vector<TokenType> &tokenTypes);
void handleNormalToken(const string &inputString, size_t &i, string &token,
                       TokenType &tokenType, std::vector<string> &tokens,
                       std::vector<TokenType> &tokenTypes);
bool handleSingleQuotedToken(const string &inputString, size_t &i,
                             string &token, TokenType &tokenType,
                             std::vector<string> &tokens,
                             std::vector<TokenType> &tokenTypes,
                             bool &multiline);
std::unordered_map<char, std::function<bool(const string &, size_t &, string &,
                                            TokenType &, std::vector<string> &,
                                            std::vector<TokenType> &, bool &)>>
    specialTokenIndicator{{'\'', handleSingleQuotedToken}};
std::unordered_set<char> concatSpeciaIndicators{'\''};

void Parser::tokenize(const string &inputString) {
  std::string token;
  TokenType tokenType;
  size_t l, r;
  l = r = skipWhitespace(inputString);

  while (r < inputString.size()) {
    if (specialTokenIndicator.contains(inputString[r])) {
      if (!specialTokenIndicator[inputString[r]](inputString, r, token,
                                                 tokenType, tokens, tokenTypes,
                                                 multiLineCommand)) {
        l = r = skipWhitespace(inputString, r);
        continue;
      }
    } else {
      handleNormalToken(inputString, r, token, tokenType, tokens, tokenTypes);
    }

    tokens.push_back(token);
    tokenTypes.push_back(tokenType);
    token = "";
    l = r = skipWhitespace(inputString, r);
  }
  // std::println("{}", tokens);
}

void handleNormalToken(const string &inputString, size_t &i, string &token,
                       TokenType &tokenType, std::vector<string> &tokens,
                       std::vector<TokenType> &tokenTypes) {
  if (concatSpeciaIndicators.contains(inputString[i - 1])) {
    concatWithPrevious(i, token, tokens, tokenTypes);
  }
  while (i < inputString.size() && !std::isspace(inputString[i]) &&
         !specialTokenIndicator.contains(inputString[i])) {
    token += inputString[i];
    i++;
  }
  token = expandToken(token);
  tokenType = TokenType::Processed;
}

bool handleSingleQuotedToken(const string &inputString, size_t &i,
                             string &token, TokenType &tokenType,
                             std::vector<string> &tokens,
                             std::vector<TokenType> &tokenTypes,
                             bool &multiline) {
  if (inputString[i + 1] == '\'') {
    i += 2;
    return false;
  }

  // concatWithPrevious
  if (!std::isspace(inputString[i - 1])) {
    concatWithPrevious(i, token, tokens, tokenTypes);
  }

  i++;
  while (inputString[i] != '\'' && i < inputString.size()) {
    token += inputString[i];
    i++;
  }
  if (i == inputString.size()) {
    multiline = true;
    return false;
  }
  i++;
  tokenType = TokenType::Literal;
  return true;
}
void concatWithPrevious(const size_t i, string &token,
                        std::vector<string> &tokens,
                        std::vector<TokenType> &tokenTypes) {
  if (tokens.empty())
    return;
  token += tokens[tokens.size() - 1];
  tokens.pop_back();
  tokenTypes.pop_back();
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
