#include "state.hpp"
#include "reading.hpp"

#include <readline/history.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void ShellState::init() {
  gatherCommands();
  rl_attempted_completion_function = contextAwareCompletion;
}

ShellState global_state;
