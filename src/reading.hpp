#pragma once

char *command_generator(const char *text, int state);
char **simpleBuiltinCompletion(const char *text, int start, int end);
