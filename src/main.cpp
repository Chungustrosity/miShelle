// #include <cctype>
#include <cstdlib>
// #include <functional>
// #include <iostream>
#include <string>
// #include <unordered_map>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>

#include "Parser.hpp"
#include "builtins.hpp"
#include "reading.hpp"
#include "redirect.hpp"

// #include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
// #include <cstring>
// #include <cstdlib>

using string = std::string;
using string_view = std::string_view;
using strVector = std::vector<std::string>;

int echo_builtin(const Parser &parsedInput);
int type_builtin(const Parser &parsedInput);
int exit_builtin(const Parser &parsedInput);
int execute_external(const Parser &parsedInput);
int pwd_builtin(const Parser &parsedInput);
int cd_builtin(const Parser &parsedInput);

CommandType findCommandType(const string &command);

int main() {
  gatherCommands();
  rl_attempted_completion_function = contextAwareCompletion;
  char *input;

  while ((input = readline("$ ")) != nullptr) {
    if (*input) {
      add_history(input);
      std::string paddedInput = std::string(" ") + input + " ";

      Parser parsedInput(paddedInput);
      string command = parsedInput.getCommand();
      strVector arguments = parsedInput.getArguments();
      if (command.empty())
        continue;

      if (isBuiltin(parsedInput)) {
        ApplyRedirectionsReturnObject redReturn =
            applyRedirections(parsedInput.getRedirections());
        int exitCode = 0;
        if (redReturn.success)
          exitCode = executeBuiltin(parsedInput);
        deApplyRedirections(redReturn);
        if (exitCode == 0)
          continue;
        if (exitCode == 1)
          break;
      } else {
        execute_external(parsedInput);
      }
    }
    free(input);
  }

  // std::vector<std::string> historyStack;
  // while (true) {
  //   cout << "$ ";
  //
  //   string fullInput;
  //   std::getline(std::cin, fullInput);
  //   fullInput = " " + fullInput + " ";
  //
  //   Parser parsedInput(fullInput, historyStack);
  //   string command = parsedInput.getCommand();
  //   strVector arguments = parsedInput.getArguments();
  //   if (command.empty())
  //     continue;
  //
  //   if (builtins.contains(command)) {
  //     ApplyRedirectionsReturnObject redReturn =
  //         applyRedirections(parsedInput.getRedirections());
  //     int exitCode = 0;
  //     if (redReturn.success)
  //       exitCode = builtins[command](parsedInput);
  //     deApplyRedirections(redReturn);
  //     if (exitCode == 0)
  //       continue;
  //     if (exitCode == 1)
  //       break;
  //   } else {
  //     execute_external(parsedInput);
  //   }
  // }
  //
  return 0;
}
