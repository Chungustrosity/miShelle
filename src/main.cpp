#include <iostream>
#include <string>
#include <vector>

std::string getInput(std::string &fullinput, std::string &command);
std::vector<std::string> splitOnSpace(std::string &str, long startIndex = 0);
int echo(std::vector<std::string> &parsedInput);
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {

    std::cout << "$ ";

    std::string fullinput, command;
    getInput(fullinput, command);
    std::vector<std::string> parsedInput = splitOnSpace(fullinput);

    if (command == "exit")
      break;
    if (command == "echo") {
      echo(parsedInput);
      continue;
    }
    std::cout << command << ": command not found\n";
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

std::vector<std::string> splitOnSpace(std::string &str, long startIndex) {
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

int echo(std::vector<std::string> &parsedInput) {
  for (int i = 1; i < parsedInput.size(); i++)
    std::cout << parsedInput[i] << " ";
  std::cout << "\n";
  return 0;
}
