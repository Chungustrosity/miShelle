#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>

#include "Parser.hpp"

namespace fs = std::filesystem;
using std::cout;
using string = std::string;
using string_view = std::string_view;
using strVector = std::vector<std::string>;

int echo_builtin(const Parser &parsedInput);
int type_builtin(const Parser &parsedInput);
int exit_builtin(const Parser &parsedInput);
int execute(const Parser &parsedInput);
int pwd_builtin(const Parser &parsedInput);
int cd_builtin(const Parser &parsedInput);

enum class CommandType { builtin, external, notACommand };
CommandType findCommandType(const string &command);

std::unordered_map<string_view, std::function<int(Parser &)>> builtins{
    {"echo", echo_builtin},
    {"type", type_builtin},
    {"exit", exit_builtin},
    {"pwd", pwd_builtin},
    {"cd", cd_builtin}};

int main() {
  std::vector<std::string> historyStack;

  while (true) {
    cout << "$ ";

    string fullInput;
    std::getline(std::cin, fullInput);
    Parser parsedInput(fullInput, historyStack);
    string command = parsedInput.getCommand();
    strVector arguments = parsedInput.getArguments();
    if (command.empty())
      continue;

    if (builtins.contains(command)) {
      int exitCode = builtins[command](parsedInput);
      if (exitCode == 0)
        continue;
      if (exitCode == 1)
        break;
    } else if (findCommandType(command) == CommandType::external) {
      execute(parsedInput);

    } else {
      cout << parsedInput.getCommand() << ": command not found\n";
    }
  }

  return 0;
}

int echo_builtin(const Parser &parsedInput) {
  for (const auto argument : parsedInput.getArguments()) {
    cout << argument << " ";
  }
  cout << "\n";
  return 0;
}

#if !defined(_WIN32)
#include <unistd.h>
#endif

bool canBeExecuted(const fs::path &filePath) {
#if defined(_WIN32)
  string extension = filePath.extension().string();
  for (char &c : extension)
    c = ::tolower(c);
  return (extension == ".exe" || extension == ".bat" || extension == ".cmd" ||
          extension == ".com");

#else
  return access(filePath.c_str(), X_OK) == 0;

#endif
}
fs::path findPathInDirectory(const string &dirPath, const string &commandName) {
  if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
    return "";
  for (const auto &dirEntry : fs::directory_iterator(dirPath)) {
    if (dirEntry.path().filename().string() == commandName &&
        canBeExecuted(dirEntry.path()))
      return dirEntry.path();
  }
  fs::path emptyPath;
  return emptyPath;
}

#if defined(_WIN32)
const char DELIM = ';';
#else
const char DELIM = ':';
#endif

fs::path findInPATH(const string &command) {
  const char *rawPath = std::getenv("PATH");
  string pathEnvVariable = (rawPath ? rawPath : "") + DELIM;
  if (pathEnvVariable == string(1, DELIM)) {
    std::cerr << "PATH Environment Variable wasn't found\n";
    return " ";
  }

  string dir;
  for (int i = 0; i < pathEnvVariable.size(); i++) {
    if (pathEnvVariable[i] == DELIM) {
      fs::path pathInDir;
      try {
        pathInDir = findPathInDirectory(dir, command);
      } catch (const fs::filesystem_error &e) {
        std::cerr << "Directory " << dir << " traversal error: " << e.what()
                  << "\n";
      }
      if (!pathInDir.empty())
        return pathInDir;

      dir = "";
    } else {
      dir += pathEnvVariable[i];
    }
  }
  fs::path emptyPath;
  return emptyPath;
}

int type_builtin(const Parser &parsedInput) {
  if (parsedInput.isNoArguments())
    return 0;

  string command = parsedInput.getArguments()[0];
  CommandType commandType = CommandType::notACommand;

  fs::path commandPath = findInPATH(command);
  if (!commandPath.empty())
    commandType = CommandType::external;

  if (builtins.contains(command)) {
    commandType = CommandType::builtin;
  }
  switch (commandType) {
  case CommandType::builtin:
    cout << command << " is a shell builtin\n";
    break;
  case CommandType::external:
    cout << command << " is " << commandPath.string() << "\n";
    break;
  case CommandType::notACommand:
    cout << command << ": not found\n";
    break;
  }
  return 0;
}

CommandType findCommandType(const string &command) {

  CommandType commandType = CommandType::notACommand;

  fs::path commandPath = findInPATH(command);
  if (!commandPath.empty())
    commandType = CommandType::external;

  if (builtins.contains(command)) {
    commandType = CommandType::builtin;
  }
  return commandType;
}
int exit_builtin(const Parser &parsedInput) { return 1; }

#if !defined(_WIN32)
#include <unistd.h>
#endif

// TODO: refactor so you don't need to check if the command is
// CommandType::external in the main loop just to apply some of the same logic
// again here to get the filePath
int execute(const Parser &parsedInput) {
  std::string command_string(parsedInput.getCommand());
  std::vector<string> args = parsedInput.get_null_terminated_args();
  std::vector<char *> argv;
  for (auto &arg : args) {
    argv.push_back(arg.data());
  }
  argv.push_back(nullptr);

  pid_t pid = fork();
  if (pid == 0) {
    if (execvp(argv[0], argv.data()) == -1) {
      std::cerr << argv[0] << "command not found\n";
      exit(1);
    }
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
  } else {
    perror("fork");
  }
  return 0;
}

int pwd_builtin(const Parser &parsedInput) {
  try {
    fs::path cwd = fs::current_path();
    cout << cwd.string() << "\n";
  } catch (const fs::filesystem_error &e) {
    std::cerr << "pwd error: " << e.what() << "\n";
  }
  return 0;
}
int cd_builtin(const Parser &parsedInput) {
  fs::path target = parsedInput.getArguments()[0];
  if (target.string()[0] == '~') {
    // only works on POSIX
    target = fs::path(std::getenv("HOME") + target.string().substr(1));
  }
  if (!fs::exists(target)) {
    cout << "cd: " << target.string() << ": No such file or directory\n";
    return 0;
  }
  // if (!fs::is_directory(target))
  //   cout << "cd: " << target.string() << ": is not a directory\n";

  // try {
  fs::current_path(target);
  // } catch (const fs::filesystem_error &e) {
  //   std::cerr << "cd error: " << e.what() << "\n";
  // }
  return 0;
}

//
