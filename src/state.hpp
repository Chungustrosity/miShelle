#pragma once

#include "Parser.hpp"
#include <filesystem>
#include <string>
#include <sys/types.h>
#include <unordered_map>

enum class JobStatus { Running, Finished };
struct Job {
  pid_t pid;
  Statement statement;
  JobStatus status = JobStatus::Running;
};

class ShellState {
private:
  int executeConditional(const Statement &statement, ShellState &state,
                         int prevExitCode);
  void executeParallel(const Statement &statement, ShellState &state);
  std::vector<Job> jobs;

public:
  void init();
  int executeParser(const Parser &parsedInput);
  int executeStatement(const Statement &statement, ShellState &state,
                       bool forkNeeded);
  const std::vector<Job> getJobs() const { return jobs; };
  std::unordered_map<std::string, std::filesystem::path> completionSpecs;
};

extern ShellState global_state;
