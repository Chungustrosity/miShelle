#include "Parser.hpp"
#include "string_utils.hpp"

#include <cctype>
#include <cstddef>
#include <functional>
// #include <print>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fcntl.h>

using std::string;
using std::string_view;
using strVector = std::vector<std::string>;

Parser::Parser(const string &fullInput
               // , const std::vector<std::string> &history
) {
  tokenize(fullInput);
  parse();
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
void tokenizeNormalToken(TokenizerState &ts);
bool tokenizeSingleQuotedToken(TokenizerState &ts);
bool tokenizeDoubleQuotedToken(TokenizerState &ts);
bool tokenizeRedirections(TokenizerState &ts);
std::unordered_map<char, std::function<bool(TokenizerState &)>>
    specialTokenIndicator{{'\'', tokenizeSingleQuotedToken},
                          {'"', tokenizeDoubleQuotedToken},
                          {'>', tokenizeRedirections}};

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
      tokenizeNormalToken(ts);
    }

    tokens.push_back(token);
    tokenTypes.push_back(tokenType);
    token = "";
    r = skipWhitespace(inputString, r);
  }
  // std::println("{}", tokens);
}

void tokenizeNormalToken(TokenizerState &ts) {
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

bool tokenizeSingleQuotedToken(TokenizerState &ts) {
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
bool tokenizeDoubleQuotedToken(TokenizerState &ts) {
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
bool tokenizeRedirections(TokenizerState &ts) {
  if (!ts.tokens.empty() && ts.inputString[ts.i - 1] != ' ' &&
      isPositiveInteger(ts.tokens[ts.tokens.size() - 1])) {
    ts.tokenTypes.pop_back();
    ts.tokenTypes.push_back(TokenType::RedirectionFileDescriptor);
  }
  ts.token += ts.inputString[ts.i];
  ts.i++;
  if (ts.inputString[ts.i] == ts.inputString[ts.i - 1]) {
    ts.token += ts.inputString[ts.i];
    ts.i++;
  }
  ts.tokenType = TokenType::Redirection;

  return true;
}
void concatWithPrevious(TokenizerState &ts) {
  if (ts.tokens.empty())
    return;
  ts.token += ts.tokens[ts.tokens.size() - 1];
  ts.tokens.pop_back();
  ts.tokenTypes.pop_back();
}

std::unordered_map<string, RedirectionType> redirectionTokenToType{
    {">", RedirectionType::OverwriteOutputRedirection},
    {">>", RedirectionType::AppendOutputRedirection},
    {"<", RedirectionType::InputRedirection}};
void initRedirection(Redirection &redi) {
  switch (redi.redirectionType) {
  case RedirectionType::OverwriteOutputRedirection:
    redi.FileDescriptor = 1;
    redi.flags = (O_WRONLY | O_CREAT | O_TRUNC);
    break;
  case RedirectionType::AppendOutputRedirection:
    redi.FileDescriptor = 1;
    redi.flags = (O_WRONLY | O_CREAT | O_APPEND);
    break;
  case RedirectionType::InputRedirection:
    redi.FileDescriptor = 0;
    redi.flags = (O_RDONLY);
  }
}

void Parser::parse() {
  if (tokens.empty())
    return;
  // parse redirections
  for (size_t i = 0; i < tokens.size(); i++) {
    if (tokenTypes[i] == TokenType::Redirection) {
      Redirection redi;
      redi.redirectionType = redirectionTokenToType[tokens[i]];
      initRedirection(redi);
      if (i > 0 && tokenTypes[i - 1] == TokenType::RedirectionFileDescriptor)
        redi.FileDescriptor = std::stoi(tokens[i - 1]);
      if (i < tokens.size() - 1 &&
          (tokenTypes[i + 1] == TokenType::Raw ||
           tokenTypes[i + 1] == TokenType::DoubleQuoted ||
           tokenTypes[i + 1] == TokenType::SingleQuoted)) {
        redi.target = tokens[i + 1];
        tokenTypes[i + 1] = TokenType::RedirectionTarget;
      }
      redirections.push_back(redi);
    }
  }
  size_t i = 0;
  // parse command
  while (command.empty() && i < tokens.size()) {
    if (command.empty() && (tokenTypes[i] == TokenType::Raw ||
                            tokenTypes[i] == TokenType::DoubleQuoted ||
                            tokenTypes[i] == TokenType::SingleQuoted)) {
      command = tokens[i];
      tokenTypes[i] = TokenType::Command;
    }
    i++;
  }
  // parse arguments
  while (i < tokens.size()) {
    if (tokenTypes[i] == TokenType::Raw ||
        tokenTypes[i] == TokenType::DoubleQuoted ||
        tokenTypes[i] == TokenType::SingleQuoted)
      arguments.push_back(tokens[i]);
    i++;
  }
}

// Public
string Parser::getCommand() const { return command; }
const strVector Parser::getArguments() const { return arguments; }
bool Parser::isNoArguments() const { return noArguments; }
bool Parser::isMultiLineCommand() const { return multiLineCommand; }
const std::vector<std::string> &Parser::getTokens() const { return tokens; }
const std::vector<TokenType> &Parser::getTokenTypes() const {
  return tokenTypes;
}
const std::vector<Redirection> &Parser::getRedirections() const {
  return redirections;
}
