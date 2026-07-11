#include "redirect.hpp"

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

ApplyRedirectionsReturnObject
applyRedirections(const std::vector<Redirection> &redirections) {
  ApplyRedirectionsReturnObject retObj;
  for (const Redirection &r : redirections) {
    int openedFileFD = open(r.target.c_str(), r.flags, 0644);
    if (openedFileFD == -1) {
      perror("open failed");
      retObj.success = false;
      return retObj;
    }

    int backup_fd = dup(r.FileDescriptor);
    retObj.appliedRedirections.push_back({r.FileDescriptor, backup_fd});

    dup2(openedFileFD, r.FileDescriptor);

    close(openedFileFD);
  }
  return retObj;
}

void deApplyRedirections(
    const ApplyRedirectionsReturnObject &redirectReturnObj) {
  for (size_t i = redirectReturnObj.appliedRedirections.size() - 1; i >= 0;
       i--) {
    const auto &backup = redirectReturnObj.appliedRedirections[i];
    dup2(backup.backupFD, backup.interceptedStreamFD);
    close(backup.backupFD);
  }
};
