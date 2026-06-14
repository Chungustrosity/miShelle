#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using parsedType = std::vector<std::string>;

std::string getInput(std::string &fullinput, std::string &command);
parsedType splitOnSpace(std::string &str, long startIndex = 0);
int echo(parsedType &parsedInput);
int type(parsedType &parsedInput);
int builtin_exit(parsedType &parsedInput);

std::unordered_map<std::string, std::function<int(parsedType &)>> builtins = {
    {"echo", echo},
    {"type", type},
    {"exit", builtin_exit},
    {"", [](parsedType) { return 0; }}

};

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {

    std::cout << "$ ";

    std::string fullinput, command;
    getInput(fullinput, command);
    parsedType parsedInput = splitOnSpace(fullinput);

    if (builtins.contains(command)) {
      int exitCode = builtins[command](parsedInput);

      if (exitCode == 1)
        break;

      continue;

    } else {
      std::cout << command << ": command not found\n";
    }
  }

  return 0;
}

int skipLeadingSpace(std::string &fullString, int startIndex = 0) {
  while (fullString[startIndex] == ' ' && startIndex < fullString.size())
    startIndex++;
  return startIndex;
}

std::string getInput(std::string &fullinput, std::string &command) {
  std::getline(std::cin, fullinput);

  long i = skipLeadingSpace(fullinput);
  while (fullinput[i] != ' ' && i < fullinput.size()) {
    command += fullinput[i];
    i++;
  }

  return fullinput;
}

parsedType splitOnSpace(std::string &str, long startIndex) {
  std::vector<std::string> words;
  long r;
  r = skipLeadingSpace(str, startIndex);

  while (r < str.size()) {
    std::string word;
    while (str[r] != ' ' && r < str.size()) {
      word += str[r];
      r++;
    }
    r = skipLeadingSpace(str, r);
    words.push_back(word);
  }
  return words;
}

int builtin_exit(parsedType &parsedInput) { return 1; }
int echo(parsedType &parsedInput) {
  for (int i = 1; i < parsedInput.size(); i++)
    std::cout << parsedInput[i] << " ";
  std::cout << "\n";
  return 0;
}

enum class CommandType {
  builtin,
  unknown,
};

int type(parsedType &parsedInput) {
  if (parsedInput.size() < 2) {
    std::cout << "type: missing operand\n";
    return 10;
  }
  std::string command = parsedInput[1];
  CommandType commandType = CommandType::unknown;

  if (builtins.contains(command))
    commandType = CommandType::builtin;

  switch (commandType) {
  case CommandType::builtin:
    std::cout << command << " is a shell builtin\n";
    break;
  case CommandType::unknown:
    std::cout << command << ": not found\n";
    break;
  }

  return 0;
}
