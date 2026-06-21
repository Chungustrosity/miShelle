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

namespace fs = std::filesystem;
using std::cout;
using string = std::string;
using string_view = std::string_view;
using strviewVector = std::vector<std::string_view>;

strviewVector splitOnSpace(const string &str);

class ParsedInput {
  strviewVector spaceSplitInput;
  string_view command;
  strviewVector arguments;
  bool noArguments = true;
  void extractArguments() {
    for (int i = 1; i < spaceSplitInput.size(); i++) {
      arguments.push_back(spaceSplitInput[i]);
    }
    if (!arguments.empty())
      noArguments = false;
  }
  void extractCommand() {
    command = spaceSplitInput.empty() ? "" : spaceSplitInput[0];
  }

public:
  ParsedInput(const string &fullInput) {
    spaceSplitInput = splitOnSpace(fullInput);
    extractCommand();
    extractArguments();
  }
  string_view getCommand() const { return command; }
  const strviewVector &getArguments() const { return arguments; }
  bool getNoArguments() const { return noArguments; }

  std::vector<string> get_null_terminated_args() const {
    std::vector<string> output;
    output.push_back(string(command));
    for (auto &arg : arguments) {
      output.push_back(string(arg));
    }
    return output;
  }
};

int echo_builtin(const ParsedInput &parsedInput);
int type_builtin(const ParsedInput &parsedInput);
int exit_builtin(const ParsedInput &parsedInput);
int ls_builtin(const ParsedInput &parsedInput);
int execute(const ParsedInput &parsedInput);
int pwd_builtin(const ParsedInput &parsedInput);
int cd_builtin(const ParsedInput &parsedInput);

enum class CommandType { builtin, external, notACommand };
CommandType findCommandType(const string_view &command);

std::unordered_map<string_view, std::function<int(ParsedInput &)>> builtins{
    {"echo", echo_builtin}, {"type", type_builtin}, {"exit", exit_builtin},
    {"pwd", pwd_builtin},   {"cd", cd_builtin},     {"ls", ls_builtin}};

int main() {

  while (true) {
    cout << "$ ";

    string fullInput;
    std::getline(std::cin, fullInput);
    ParsedInput parsedInput(fullInput);
    string_view command = parsedInput.getCommand();
    strviewVector arguments = parsedInput.getArguments();
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

size_t skipWhitespace(const string &str, size_t startIndex = 0) {
  size_t i = startIndex;
  while (i < str.size() && std::isspace(str[i]))
    i++;
  return i;
}
strviewVector splitOnSpace(const string &fullText) {
  strviewVector words;
  std::string_view fullText_view(fullText);
  size_t l, r;
  l = r = skipWhitespace(fullText);
  while (l < fullText_view.size()) {
    while (r < fullText_view.size() && !std::isspace(fullText_view[r]))
      r++;

    std::string_view word = fullText_view.substr(l, r - l);
    words.push_back(word);

    l = r = skipWhitespace(fullText, r);
  }
  return words;
}

int echo_builtin(const ParsedInput &parsedInput) {
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
  for (char *c : extension)
    c = ::tolower(c);
  return (extension == ".exe" && extension == ".bat" && extension == ".cmd" &&
          extension == ".com")

#else
  return access(filePath.c_str(), X_OK) == 0;

#endif
}
fs::path findPathInDirectory(const string_view &dirPath,
                             const string_view commandName) {
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

fs::path findInPATH(const string_view &command) {
  const char *pathEnvVariable = std::getenv("PATH");
  if (pathEnvVariable == nullptr) {
    std::cerr << "PATH Environment Variable wasn't found\n";
    return " ";
  }
  string_view pathEnvVariable_view = std::getenv("PATH");

#if defined(_WIN32)
  const char DELIM = ';';
#else
  const char DELIM = ':';
#endif
  size_t l;
  l = 0;
  while (l != std::string_view::npos) {
    size_t r = pathEnvVariable_view.find(DELIM, l);

    string_view dir = pathEnvVariable_view.substr(l, r - l);
    fs::path pathInDir = findPathInDirectory(dir, command);
    if (!pathInDir.empty())
      return pathInDir;

    l = r == string::npos ? string::npos : r + 1;
  }
  fs::path emptyPath;
  return emptyPath;
}

int type_builtin(const ParsedInput &parsedInput) {
  if (parsedInput.getNoArguments())
    return 0;

  string_view command = parsedInput.getArguments()[0];
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

CommandType findCommandType(const string_view &command) {

  CommandType commandType = CommandType::notACommand;

  fs::path commandPath = findInPATH(command);
  if (!commandPath.empty())
    commandType = CommandType::external;

  if (builtins.contains(command)) {
    commandType = CommandType::builtin;
  }
  return commandType;
}
int exit_builtin(const ParsedInput &parsedInput) { return 1; }
int ls_builtin(const ParsedInput &parsedInput) { return 0; }

#if !defined(_WIN32)
#include <unistd.h>
#endif

// TODO: refactor so you don't need to check if the command is
// CommandType::external in the main loop just to apply some of the same logic
// again here to get the filePath
int execute(const ParsedInput &parsedInput) {
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

int pwd_builtin(const ParsedInput &parsedInput) {
  try {
    fs::path cwd = fs::current_path();
    cout << cwd.string() << "\n";
  } catch (const fs::filesystem_error &e) {
    std::cerr << "pwd error: " << e.what() << "\n";
  }
  return 0;
}
int cd_builtin(const ParsedInput &parsedInput) {
  fs::path target = parsedInput.getArguments()[0];
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
