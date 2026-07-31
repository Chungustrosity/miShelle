#include "state.hpp"
#include "Parser.hpp"
#include "builtins.hpp"
#include "reading.hpp"
#include "redirect.hpp"

#include <readline/history.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ShellState global_state;

void ShellState::init() {
  gatherCommands();
  rl_attempted_completion_function = contextAwareCompletion;
}
void ShellState::executeParser(const Parser &parsedInput) {
  const auto &terminators = parsedInput.getStatementTerminators();
  const auto &statements = parsedInput.getStatements();
  int lastExitCode = 0;
  for (int statementIndex = 0; statementIndex < terminators.size();
       statementIndex++) {
    const auto &statement = statements[statementIndex];

    switch (terminators[statementIndex]) {
    case CommandTerminatorType::Sequence:
      lastExitCode = executeStatement(statement, global_state);
      break;
    case CommandTerminatorType::And:
      lastExitCode = executeConditional(statement, global_state, lastExitCode);
      break;
    case CommandTerminatorType::Async:
      executeParallel(statement, global_state);
      break;
    }
  }
}

int ShellState::executeStatement(const Statement &statement,
                                 ShellState &state) {
  std::string command = statement.command;
  CommandType commandType = findCommandType(command);
  switch (commandType) {
  case CommandType::builtin: {
    const auto &redirections = applyRedirections(statement.redirections);
    int exitCode = 0;
    if (redirections) {
      exitCode = executeBuiltinCommandStatement(statement, state);
      deApplyRedirections(*redirections);
    }
    return exitCode;
  }

  case CommandType::notACommand:
  case CommandType::external:
    return execute_external(statement, global_state);
  }
}

int ShellState::executeConditional(const Statement &statement,
                                   ShellState &state, int prevExitCode) {
  if (prevExitCode == 0) {
    return executeStatement(statement, state);
  }
  return prevExitCode;
}

void ShellState::executeParallel(const Statement &statement,
                                 ShellState &state) {
  Job job;
  job.statement = statement;
  job.pid = fork();
  if (job.pid == 0) {
    int exitCode = executeStatement(statement, state);
    exit(exitCode);
  } else if (job.pid > 0) {
    jobs.push_back(job);
  } else {
    perror("fork");
  }
}
