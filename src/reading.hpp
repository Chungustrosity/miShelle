#pragma once

char *command_generator(const char *text, int state);
char **contextAwareCompletion(const char *text, int start, int end);
