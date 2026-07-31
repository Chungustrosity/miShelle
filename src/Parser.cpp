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
  std::vector<CommandTerminatorType> &statementTerminators;
};
void concatWithPrevious(TokenizerState &ts);
void tokenizeNormalToken(TokenizerState &ts);
bool tokenizeSingleQuotedToken(TokenizerState &ts);
bool tokenizeDoubleQuotedToken(TokenizerState &ts);
bool tokenizeRedirections(TokenizerState &ts);
bool tokenizeCommandTerminator(TokenizerState &ts);
std::unordered_map<char, std::function<bool(TokenizerState &)>>
    specialTokenIndicator{{'\'', tokenizeSingleQuotedToken},
                          {'"', tokenizeDoubleQuotedToken},
                          {'>', tokenizeRedirections},
                          {';', tokenizeCommandTerminator},
                          {'&', tokenizeCommandTerminator}};

std::unordered_set<char> concatSpeciaIndicators{'\'', '"'};

std::unordered_map<std::string, CommandTerminatorType> terminatorTokenToType{
    {"&", CommandTerminatorType::Async},
    {";", CommandTerminatorType::Sequence},
    {"&&", CommandTerminatorType::And}};

std::unordered_map<string, RedirectionType> redirectionTokenToType{
    {">", RedirectionType::OverwriteOutputRedirection},
    {">>", RedirectionType::AppendOutputRedirection},
    {"<", RedirectionType::InputRedirection}};

void Parser::tokenize(const string &inputString) {
  std::string token;
  TokenType tokenType;
  size_t r;
  TokenizerState ts(inputString, r, token, tokenType, tokens, tokenTypes,
                    multiLineCommand, statementTerminators);
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
  // could be moved to the parsing phase
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
bool tokenizeCommandTerminator(TokenizerState &ts) {
  ts.token += ts.inputString[ts.i];
  ts.tokenType = TokenType::CommandTerminator;
  ts.i++;
  if (ts.inputString[ts.i] == ts.inputString[ts.i - 1] &&
      ts.inputString[ts.i] == '&') {
    ts.token += ts.inputString[ts.i];
    ts.i++;
  }
  if (terminatorTokenToType.contains(ts.token))
    ts.statementTerminators.push_back(terminatorTokenToType[ts.token]);

  return true;
}
void concatWithPrevious(TokenizerState &ts) {
  if (ts.tokens.empty())
    return;
  ts.token += ts.tokens[ts.tokens.size() - 1];
  ts.tokens.pop_back();
  ts.tokenTypes.pop_back();
}

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

int Parser::parseStatement(int statementNumber, Statement &statementObject) {

  statementObject.terminator = statementTerminators[statementNumber];

  StatementSpan span = statementSpans[statementNumber];
  int statementFirstIndex = span.start; // inclusive
  int statementLastIndex = span.end;    // exclusive

  // parse redirections
  redirections.push_back({});
  for (size_t i = statementFirstIndex; i < statementLastIndex; i++) {
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
        // tokenTypes[i + 1] = TokenType::RedirectionTarget;
        semanticTags[i + 1] = SemanticTag::RedirectionTarget;
      }
      redirections[statementNumber].push_back(redi);
      statementObject.redirections.push_back(redi);
    }
  }
  // parse command
  size_t i = statementFirstIndex;
  commands.push_back({});
  while (
      (commands[statementNumber].empty() || statementObject.command.empty()) &&
      i < statementLastIndex) {
    // TODO: check whether the first condition in the if statement is necessary
    // with tests maybe
    if (commands[statementNumber].empty() &&
        (tokenTypes[i] == TokenType::Raw ||
         tokenTypes[i] == TokenType::DoubleQuoted ||
         tokenTypes[i] == TokenType::SingleQuoted) &&
        semanticTags[i] != SemanticTag::RedirectionTarget) {

      commands[statementNumber] = tokens[i];
      statementObject.command = tokens[i];
      // tokenTypes[i] = TokenType::Command;
      semanticTags[i] = SemanticTag::Command;
    }
    i++;
  }
  // parse arguments
  arguments.push_back({});
  while (i < statementLastIndex) {
    if ((tokenTypes[i] == TokenType::Raw ||
         tokenTypes[i] == TokenType::DoubleQuoted ||
         tokenTypes[i] == TokenType::SingleQuoted) &&
        semanticTags[i] != SemanticTag::RedirectionTarget) {
      arguments[statementNumber].push_back(tokens[i]);
      statementObject.arguments.push_back(tokens[i]);
    }
    i++;
  }

  return 0;
}
bool Parser::populateStatementSpans() {
  int statementStart = 0;
  int statementEnd;
  for (int i = 0; i < tokenTypes.size(); i++) {
    const TokenType &tt = tokenTypes[i];
    switch (tt) {
    case TokenType::CommandTerminator:
      semanticTags.push_back(SemanticTag::CommandTerminator);
      statementEnd = i;
      if (statementEnd - statementStart <= 0) {
        // TODO: handle invalid syntax
        return false;
      } else {
        statementSpans.push_back(StatementSpan(statementStart, statementEnd));
        statementStart = i + 1;
      }
      break;
    default:
      semanticTags.push_back(SemanticTag::Argument);
    }
  }
  // handling implied trailing terminator
  if (statementStart < tokenTypes.size()) {
    statementSpans.push_back(StatementSpan(statementStart, tokenTypes.size()));
    statementTerminators.push_back(CommandTerminatorType::Sequence);
  }
  return true;
}
void Parser::parse() {
  // populating `statementSpans` could probably be done in the tokenization
  // phase as an optimization

  if (tokens.empty())
    return;

  // populates `statementSpans`. And populates `semanticTags` with
  // SemanticTag::Argument
  if (populateStatementSpans() == false)
    return; // invalid syntax case
  for (int statementNumber = 0; statementNumber < statementSpans.size();
       statementNumber++) {
    Statement statement;
    parseStatement(statementNumber, statement);
    statements.push_back(statement);
  }
}

// Public
string Parser::getCommand(int commandStatement) const {
  return commands[commandStatement];
}
const strVector Parser::getArguments(int commandStatement) const {
  return arguments[commandStatement];
}
bool Parser::isMultiLineCommand() const { return multiLineCommand; }
// const std::vector<std::string> &Parser::getTokens() const { return tokens; }
// const std::vector<TokenType> &Parser::getTokenTypes() const {
//   return tokenTypes;
// }
// const std::vector<SemanticTag> &Parser::getSemanticTags() const {
//   return semanticTags;
// }
const std::vector<Redirection> &
Parser::getRedirections(int commandStatement) const {
  return redirections[commandStatement];
}
