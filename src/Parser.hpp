#pragma once

#include "redirect.hpp"

// Raw, SingleQuoted, and DoubleQuoted are intermediate token types that may
// resolve to arguments and commands. <---- outdated lol
enum class TokenType {
  Raw,
  DoubleQuoted,
  SingleQuoted,
  Redirection,
  RedirectionFileDescriptor,
  CommandTerminator
};
enum class SemanticTag {
  Command,
  Argument,
  CommandTerminator,
  Redirection,
  RedirectionFileDescriptor,
  RedirectionTarget,
  CommandTermindator
};
enum class CommandTerminatorType { Sequence, Async, And };

struct StatementSpan {
  const int start;
  const int end; // exclusive
  StatementSpan(int a, int b) : start(a), end(b) {}
};
struct Statement {
  std::string command;
  std::vector<std::string> arguments;
  std::vector<Redirection> redirections;
  bool noArguments = false;
  CommandTerminatorType terminator = CommandTerminatorType::Sequence;

  std::string reconstructCommand() const {
    std::string recCmd = command;
    for (const std::string &arg : arguments)
      recCmd += " " + arg;
    // TODO: append redirections
    return recCmd;
  }
};

class Parser {
private:
  std::vector<StatementSpan> statementSpans;
  std::vector<CommandTerminatorType> statementTerminators;
  std::vector<Statement> statements;
  std::vector<std::string> commands;
  std::vector<std::vector<std::string>> arguments;
  std::vector<std::vector<Redirection>> redirections;
  std::vector<bool> noArguments;
  bool multiLineCommand = false;

  std::vector<std::string> tokens;
  std::vector<TokenType> tokenTypes;
  std::vector<SemanticTag> semanticTags;

  void tokenize(const std::string &inputString);
  void parse();
  int parseStatement(int statementNumber, Statement &statementObj);
  bool populateStatementSpans();

public:
  Parser(const std::string &fullInput
         // , const std::vector<std::string> &history
  );

  std::string getCommand(int commandStatement) const;
  const std::vector<std::string> getArguments(int commandStatement) const;
  bool isNoArguments(int commandStatement) const {
    return (statements[commandStatement].arguments.size() == 0);
  };
  bool isMultiLineCommand() const;
  const std::vector<std::string> &getTokens() const { return tokens; };
  const std::vector<TokenType> &getTokenTypes() const { return tokenTypes; };
  const std::vector<SemanticTag> &getSemanticTags() const {
    return semanticTags;
  };
  const std::vector<CommandTerminatorType> &getStatementTerminators() const {
    return statementTerminators;
  };
  const std::vector<Redirection> &getRedirections(int commandStatement) const;
  const int getLastStatement() const { return statements.size() - 1; }
  const std::vector<Statement> &getStatements() const { return statements; };
};
