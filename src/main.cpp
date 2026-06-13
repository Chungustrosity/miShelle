#include <iostream>
#include <string>

std::string getInput(std::string &fullinput, std::string &command);
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::string builtins[] = {"exit", "ls"};

  while (true) {

    std::cout << "$ ";

    std::string fullinput, command;
    getInput(fullinput, command);
    // std::cout << "fullinput: " << fullinput << "\ncommand: " << command
    //           << std::endl;

    if (command == "exit")
      break;
    std::cout << command << ": command not found\n";
  }

  return 0;
}

std::string getInput(std::string &fullinput, std::string &command) {
  std::getline(std::cin, fullinput);

  long i = 0;
  // skipping leading whitespace
  while (fullinput[i] == ' ' && i < fullinput.size())
    i++;
  while (fullinput[i] != ' ' && i < fullinput.size()) {
    command += fullinput[i];
    i++;
  }

  return fullinput;
}
