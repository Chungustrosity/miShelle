#pragma once

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
};
struct ApplyRedirectionsReturnObject {
  bool success = true;
  std::vector<SavedFD> appliedRedirections;
};

ApplyRedirectionsReturnObject
applyRedirections(const std::vector<Redirection> &redirections);

void deApplyRedirections(
    const ApplyRedirectionsReturnObject &redirectReturnObj);
