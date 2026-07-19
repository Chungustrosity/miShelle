#include <cstdlib>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "Parser.hpp"
#include "builtins.hpp"
#include "redirect.hpp"
#include "state.hpp"

using string = std::string;
using string_view = std::string_view;
using strVector = std::vector<std::string>;

CommandType findCommandType(const string &command);

int main() {
  global_state.init();
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
          exitCode = executeBuiltin(parsedInput, global_state);
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
