#include <cstdlib>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "Parser.hpp"
#include "builtins.hpp"
#include "reading.hpp"
#include "redirect.hpp"

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
  return 0;
}
