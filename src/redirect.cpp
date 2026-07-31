#include "redirect.hpp"

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

std::optional<std::vector<SavedFD>>
applyRedirections(const std::vector<Redirection> &redirections) {
  std::vector<SavedFD> retVec;
  for (const Redirection &r : redirections) {
    int openedFileFD = open(r.target.c_str(), r.flags, 0644);
    if (openedFileFD == -1) {
      perror("open failed");
      deApplyRedirections(retVec);
      return std::nullopt;
    }

    int backup_fd = dup(r.FileDescriptor);
    if (backup_fd == -1) {
      perror("dup failed");
      close(openedFileFD);
      deApplyRedirections(retVec);
      return std::nullopt;
    }

    retVec.push_back({r.FileDescriptor, backup_fd});

    if (dup2(openedFileFD, r.FileDescriptor) == -1) {
      perror("dup2 failed");
      close(openedFileFD);
      deApplyRedirections(retVec);
      return std::nullopt;
    }

    if (openedFileFD != r.FileDescriptor) {
      close(openedFileFD);
    }
  }
  return retVec;
}

void deApplyRedirections(const std::vector<SavedFD> &redirections) {
  for (int i = redirections.size() - 1; i >= 0; i--) {
    const auto &backup = redirections[i];
    dup2(backup.backupFD, backup.interceptedStreamFD);
    close(backup.backupFD);
  }
};
