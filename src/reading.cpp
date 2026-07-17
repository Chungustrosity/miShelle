#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <readline/readline.h>

std::vector<std::string> builtins_list{"echo", "type", "exit", "pwd", "cd"};

char *command_generator(const char *text, int state) {
  static size_t list_index;
  static int len;

  if (state == 0) {
    list_index = 0;
    len = strlen(text);
  }

  while (list_index < builtins_list.size()) {
    const std::string &name = builtins_list[list_index];
    list_index++;
    if (strncmp(name.c_str(), text, len) == 0) {
      return strdup(name.c_str());
    }
  }
  return nullptr;
}
char **simpleBuiltinCompletion(const char *text, int start, int end) {
  if (start == 0) {
    return rl_completion_matches(text, command_generator);
  }
  return nullptr;
}
