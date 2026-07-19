#include "reading.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#include <readline/readline.h>

#include "Parser.hpp"
#include "builtins.hpp"
#include "state.hpp"

namespace fs = std::filesystem;

std::unordered_set<std::string> uniqueCommands;

void appendBuiltinsToSet(std::unordered_set<std::string> &uniqueCommands) {
  for (const auto &[commandName, commandFunction] : builtins) {
    uniqueCommands.insert(commandName);
  }
}
void pushViableCommandsToSet(const std::string &dirPath,
                             std::unordered_set<std::string> &commandSet) {
  if (dirPath.empty() || !fs::exists(dirPath) || !fs::is_directory(dirPath))
    return;
  for (const auto &dirEntry : fs::directory_iterator(dirPath)) {
    if (fs::is_regular_file(dirEntry.status())) {
      if (access(dirEntry.path().c_str(), X_OK) == 0) {
        commandSet.insert(dirEntry.path().filename().string());
      }
    }
  }
}
void gatherCommands() {
  const char DELIM = ':';
  const char *rawPATH = std::getenv("PATH");
  if (!rawPATH) {
    return;
  }
  std::string PATH = std::string(rawPATH) + DELIM;
  size_t l = 0, r = 0;

  if (PATH == std::string(1, DELIM)) {
    std::cerr << "PATH Environment Variable wasn't found\n";
    // return " ";
  }

  std::string dir;
  for (size_t i = 0; i < PATH.size(); i++) {
    if (PATH[i] == DELIM) {
      try {
        pushViableCommandsToSet(dir, uniqueCommands);
      } catch (const fs::filesystem_error &e) {
        std::cerr << "Directory " << dir << " traversal error: " << e.what()
                  << "\n";
      }
      dir = "";
    } else {
      dir += PATH[i];
    }
  }
  appendBuiltinsToSet(uniqueCommands);
}

extern char *rl_line_buffer;
extern int rl_point;

char *command_generator(const char *text, int state);
char *spec_argument_generator(const char *text, int state);

char **contextAwareCompletion(const char *text, int start, int end) {
  std::string inputSoFar(rl_line_buffer, rl_point);
  std::string paddedInput = " " + inputSoFar + "dummy" + " ";
  Parser parser(paddedInput);
  const auto &tokens = parser.getTokens();
  const auto &tokenTypes = parser.getTokenTypes();
  TokenType currentTokenType = tokenTypes.back();
  switch (currentTokenType) {
  case TokenType::Command:
    return rl_completion_matches(text, command_generator);
  case TokenType::Raw:
  case TokenType::DoubleQuoted:
  case TokenType::SingleQuoted:
    if (global_state.completionSpecs.contains(parser.getCommand())) {
      return rl_completion_matches(text, spec_argument_generator);
    }

  case TokenType::RedirectionTarget:

  // impossible
  case TokenType::RedirectionFileDescriptor:
  case TokenType::Redirection:

  default:
    return nullptr;
  }
}

char *command_generator(const char *text, int state) {
  static std::unordered_set<std::string>::const_iterator it;
  static int len;

  if (state == 0) {
    it = uniqueCommands.begin();
    len = strlen(text);
  }

  while (it != uniqueCommands.end()) {
    const std::string &command = *it;
    it++;
    if (strncmp(command.c_str(), text, len) == 0)
      return strdup(command.c_str());
  }
  return nullptr;
}

std::unordered_set<std::string> fetch_suggestions();
std::vector<std::string> getExecArgs();

char *spec_argument_generator(const char *text, int state) {

  static std::unordered_set<std::string> suggestions;
  static std::unordered_set<std::string>::const_iterator it;
  static int len;

  if (state == 0) {
    suggestions = fetch_suggestions();
    it = suggestions.begin();
    len = strlen(text);
  }

  while (it != suggestions.end()) {
    const std::string &suggestion = *it;
    it++;
    if (strncmp(suggestion.c_str(), text, len) == 0)
      return strdup(suggestion.c_str());
  }

  return nullptr;
}

std::unordered_set<std::string> fetch_suggestions() {
  std::unordered_set<std::string> result;
  std::vector<std::string> args = getExecArgs();
  std::vector<char *> argv;
  for (std::string &arg : args)
    argv.push_back(arg.data());
  argv.push_back(nullptr);

  const std::filesystem::path &path = global_state.completionSpecs[argv[0]];
  const char *path_c = path.c_str();

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("shell: pipe failed");
    return result;
  }
  pid_t pid = fork();

  if (pid == -1) {
    perror("shell: fork failed");
    close(pipefd[0]);
    close(pipefd[1]);
  } else if (pid == 0) {
    close(pipefd[0]);
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
      perror("child process: dup2 failed, internal failure in reading "
             "autocompletion suggestions");
      exit(EXIT_FAILURE);
    }
    close(pipefd[1]);
    setenv("COMP_LINE", rl_line_buffer, 1);
    setenv("COMP_POINT", std::to_string(rl_point).c_str(), 1);
    execv(path_c, argv.data());
    perror("shell: execv failed");
    exit(EXIT_FAILURE);
  } else {
    close(pipefd[1]);
    FILE *pipe_stream = fdopen(pipefd[0], "r");
    if (!pipe_stream) {
      perror("shell: fdopen failed");
      return result;
    }

    char *line_buffer = nullptr;
    size_t line_buffer_size = 0;
    ssize_t bytes_read;

    while ((bytes_read =
                getline(&line_buffer, &line_buffer_size, pipe_stream)) != -1) {
      std::string completionLine(line_buffer, bytes_read);
      if (!completionLine.empty() && completionLine.back() == '\n')
        completionLine.pop_back();
      if (!completionLine.empty())
        result.insert(completionLine);
    }
    free(line_buffer);
    fclose(pipe_stream);
    waitpid(pid, nullptr, 0);
  }
  return result;
}
std::vector<std::string> getExecArgs() {
  std::vector<std::string> argv;
  std::string inputSoFar(rl_line_buffer, rl_point);
  bool isCurrentTokenEmpty =
      rl_line_buffer[rl_point] == ' '; // doule check if valid
  Parser parser(inputSoFar);
  argv.push_back(parser.getCommand());
  auto args = parser.getArguments();

  if (isCurrentTokenEmpty) {
    argv.push_back("");
  } else {
    argv.push_back(args.back());
  }

  if (args.size() == 0) {
    argv.push_back("");
  } else if (args.size() == 1) {
    argv.push_back("");
  } else if (args.size() >= 2) {
    argv.push_back(args[args.size() - 2]);
  }

  return argv;
}
