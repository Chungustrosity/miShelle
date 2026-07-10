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

struct TokenizerState {
  const string &inputString;
  size_t &i;
  string &token;
  TokenType &tokenType;
  std::vector<string> &tokens;
  std::vector<TokenType> &tokenTypes;
  bool &multiLineCommand;
};
void concatWithPrevious(TokenizerState &ts);
void handleNormalToken(TokenizerState &ts);
bool handleSingleQuotedToken(TokenizerState &ts);
bool handleDoubleQuotedToken(TokenizerState &ts);
bool handleRedirections(TokenizerState &ts);
std::unordered_map<char, std::function<bool(TokenizerState &)>>
    specialTokenIndicator{{'\'', handleSingleQuotedToken},
                          {'"', handleDoubleQuotedToken},
                          {'>', handleRedirections}};

std::unordered_set<char> concatSpeciaIndicators{'\'', '"'};

void Parser::tokenize(const string &inputString) {
  std::string token;
  TokenType tokenType;
  size_t r;
  TokenizerState ts(inputString, r, token, tokenType, tokens, tokenTypes,
                    multiLineCommand);
  r = skipWhitespace(inputString);

  while (r < inputString.size()) {
    if (specialTokenIndicator.contains(inputString[r])) {
      if (!specialTokenIndicator[inputString[r]](ts)) {
        r = skipWhitespace(inputString, r);
        continue;
      }
    } else {
      handleNormalToken(ts);
    }

    tokens.push_back(token);
    tokenTypes.push_back(tokenType);
    token = "";
    r = skipWhitespace(inputString, r);
  }
  // std::println("{}", tokens);
}

void handleNormalToken(TokenizerState &ts) {
  if (concatSpeciaIndicators.contains(ts.inputString[ts.i - 1])) {
    concatWithPrevious(ts);
  }
  while (ts.i < ts.inputString.size() && !std::isspace(ts.inputString[ts.i]) &&
         !specialTokenIndicator.contains(ts.inputString[ts.i])) {
    if (ts.inputString[ts.i] == '\\' &&
        ts.i < ts.inputString.size() - 2) /* -2 because the input is padded */ {
      ts.token += ts.inputString[ts.i + 1];
      ts.i += 2;
      continue;
    }
    ts.token += ts.inputString[ts.i];
    ts.i++;
  }
  ts.token = expandToken(ts.token);
  ts.tokenType = TokenType::Raw;
}

bool handleSingleQuotedToken(TokenizerState &ts) {
  if (ts.inputString[ts.i + 1] == '\'') {
    ts.i += 2;
    return false;
  }

  // concatWithPrevious
  if (!std::isspace(ts.inputString[ts.i - 1])) {
    concatWithPrevious(ts);
  }

  ts.i++;
  while (ts.inputString[ts.i] != '\'' && ts.i < ts.inputString.size()) {
    ts.token += ts.inputString[ts.i];
    ts.i++;
  }
  if (ts.i == ts.inputString.size()) {
    ts.multiLineCommand = true;
    return false;
  }
  ts.i++;
  ts.tokenType = TokenType::SingleQuoted;
  return true;
}
bool handleDoubleQuotedToken(TokenizerState &ts) {
  if (ts.inputString[ts.i + 1] == '"') {
    ts.i += 2;
    return false;
  }

  // concatWithPrevious
  if (!std::isspace(ts.inputString[ts.i - 1])) {
    concatWithPrevious(ts);
  }

  ts.i++;
  while (ts.inputString[ts.i] != '"' && ts.i < ts.inputString.size()) {
    if (ts.inputString[ts.i] == '\\' &&
        ts.i <
            ts.inputString.size() -
                3) /* -3 because you can't escape the closing quote (in the edge
                      case that it's the last token, and \ is the last char) */
    {
      if (ts.inputString[ts.i + 1] == '"' || ts.inputString[ts.i + 1] == '\\') {
        ts.token += ts.inputString[ts.i + 1];
        ts.i += 2;
        continue;
      }
    }
    ts.token += ts.inputString[ts.i];
    ts.i++;
  }
  if (ts.i == ts.inputString.size()) {
    ts.multiLineCommand = true;
    return false;
  }
  ts.i++;
  ts.tokenType = TokenType::DoubleQuoted;
  return true;
}
bool handleRedirections(TokenizerState &ts) { return true; }
void concatWithPrevious(TokenizerState &ts) {
  if (ts.tokens.empty())
    return;
  ts.token += ts.tokens[ts.tokens.size() - 1];
  ts.tokens.pop_back();
  ts.tokenTypes.pop_back();
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
