#pragma once

#include <optional>
#include <string>
#include <vector>

enum class RedirectionType {
  OverwriteOutputRedirection,
  AppendOutputRedirection,
  InputRedirection
};

struct Redirection {
  RedirectionType redirectionType;
  int FileDescriptor;
  std::string target;
  int flags = 0;
};
struct SavedFD {
  int interceptedStreamFD;
  int backupFD;
  SavedFD(int a, int b) : interceptedStreamFD(a), backupFD(b) {};
};

std::optional<std::vector<SavedFD>>
applyRedirections(const std::vector<Redirection> &redirections);

void deApplyRedirections(const std::vector<SavedFD> &redirections);
