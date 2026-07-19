#include "builtins.hpp"
#include "state.hpp"

#include <filesystem>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <sys/wait.h>

using std::cout;

const std::unordered_map<string,
                         std::function<int(const Parser &, ShellState &)>>
    builtins{{"echo", echo_builtin}, {"type", type_builtin},
             {"exit", exit_builtin}, {"pwd", pwd_builtin},
             {"cd", cd_builtin},     {"complete", complete_builtin}};

bool isBuiltin(const Parser &parsedInput) {
  return builtins.contains(parsedInput.getCommand());
}
int executeBuiltin(const Parser &parsedInput, ShellState &state) {
  return builtins.at(parsedInput.getCommand())(parsedInput, state);
}

int echo_builtin(const Parser &parsedInput, ShellState &state) {
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

fs::path findInPATH(const string &command) {
  const char *rawPath = std::getenv("PATH");
  string pathEnvVariable = string(rawPath ? rawPath : "") + DELIM;
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

int type_builtin(const Parser &parsedInput, ShellState &state) {
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
int exit_builtin(const Parser &parsedInput, ShellState &state) { return 1; }

#if !defined(_WIN32)
#include <unistd.h>
#endif

int execute_external(const Parser &parsedInput) {
  std::string command_string(parsedInput.getCommand());
  std::vector<string> args = parsedInput.getArguments();
  std::vector<char *> argv;
  argv.push_back(command_string.data());
  for (auto &arg : args) {
    argv.push_back(arg.data());
  }
  argv.push_back(nullptr);

  pid_t pid = fork();
  if (pid == 0) {
    applyRedirections(parsedInput.getRedirections());
    if (execvp(argv[0], argv.data()) == -1) {
      std::cerr << argv[0] << ": command not found\n";
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

int pwd_builtin(const Parser &parsedInput, ShellState &state) {
  try {
    fs::path cwd = fs::current_path();
    cout << cwd.string() << "\n";
  } catch (const fs::filesystem_error &e) {
    std::cerr << "pwd error: " << e.what() << "\n";
  }
  return 0;
}
int cd_builtin(const Parser &parsedInput, ShellState &state) {
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

int complete_builtin(const Parser &parsedInput, ShellState &state) {
  const std::vector<std::string> &args = parsedInput.getArguments();
  if (args.size() < 2)
    return 0;
  std::string flag = args[0];
  if (flag == "-p") {
    if (!state.completionSpecs.contains(args[1])) {
      cout << "complete: " << args[1] << ": no completion specification"
           << std::endl;
    } else {
      cout << "complete -C \'" << state.completionSpecs[args[1]].string()
           << "\' " << args[1] << std::endl;
    }
  } else if (args.size() > 3 && flag == "-C") {
    state.completionSpecs[args[2]] = fs::path(args[1]);
  } else if (flag == "-r") {
    state.completionSpecs.erase(args[1]);
  }

  return 0;
}
