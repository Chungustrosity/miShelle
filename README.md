# MiShelle

MiShelle is a small interactive shell written in C++23 for [CodeCrafters' Build Your Own Shell challenge](https://app.codecrafters.io/courses/shell/overview). It runs builtins and programs from `PATH`, parses quotes and output redirections, and handles multiple statements on one line. It is a learning project, not a POSIX-compliant shell.

The name is a pun on “my shell” said with a British accent. A shell with a feminine name was also an amusing image because shells spawn *child* processes.

## Build and run

MiShelle currently targets a Unix-like environment. You need a C++23 compiler, CMake 3.15 or newer, and GNU Readline development headers and library. CMake fetches GoogleTest on first configuration, which may require network access.

```sh
cmake -S . -B build
cmake --build build
./build/shell
```

`./your_program.sh` is the CodeCrafters launcher. It also builds and runs the shell, but expects `VCPKG_ROOT` to point to a vcpkg installation. The direct CMake commands above do not use that variable.

```text
$ echo hello world
hello world
$ type echo
echo is a shell builtin
$ echo one; echo two
one
two
$ exit
```

## Features

- External commands run through `fork` and `execvp`. `type` reports whether a name is a builtin or an executable found in `PATH`.
- Builtins: `echo`, `pwd`, `cd`, `type`, `exit`, `complete`, and `jobs`. `cd` changes the shell's own working directory.
- The parser handles single and double quotes, adjacent quoted and unquoted text, backslash escapes, and basic tilde expansion.
- `>` and `>>` redirect external command output. An explicit file descriptor, such as `2>errors.txt`, is supported.
- `;` executes statements in sequence, and `&` launches a statement in the background. `jobs` displays the jobs recorded in the current session.
- GNU Readline provides line editing, history, and tab completion. Command completion uses builtins and executable names collected from `PATH`.

The `complete` builtin can register a programmable argument-completion command with `complete -C /path/to/program command`. The intended interface passes the command name, current word, and previous word as arguments, sets `COMP_LINE` and `COMP_POINT`, and reads one suggestion per output line. `complete -p command` shows a registration; `complete -r command` removes it. Registrations last only for the current session. The interactive completion path is currently unreliable; see **Current limits**.

## Architecture

```text
Readline input (src/main.cpp)
  → tokenization and parsing (src/Parser.cpp)
  → statement dispatch and job storage (src/state.cpp)
  → builtin or external execution (src/builtins.cpp)
  → optional descriptor redirection (src/redirect.cpp)
```

`src/reading.cpp` handles completion, and `src/string_utils.cpp` contains parsing helpers. The parser produces `Statement` objects with a command, arguments, redirections, and a terminator. `ShellState` executes them and retains completion specifications and background job records.

## How it grew

The diffs show the project's gradual change from a single-file challenge solution into a modular shell:

1. **June 12–21:** The first commits built the prompt, `echo`, `type`, and external execution in `src/main.cpp`. `pwd` and `cd` followed. Early CodeCrafters submission commits sometimes contain several changes at once.
2. **June 27–July 10:** Work on quoting exposed the limits of splitting input on spaces. The parser and string helpers moved to separate files so the growing code could be tested. Tests, double quotes, escapes, and redirection parsing followed.
3. **July 11–20:** Redirection began applying to actual file descriptors. GNU Readline completion started with builtin names, expanded to executables in `PATH`, then gained context-aware programmable completion. Builtins, completion, and session state moved out of `main.cpp`.
4. **July 21–31:** Work on `jobs` led to a multi-statement parser and a larger execution refactor. `ShellState` began dispatching statements and tracking background jobs, leaving `main.cpp` as the input loop.

This timeline comes from the code changes, not just the commit titles. The current source and tests remain the reference for what works now.

## Current limits

- `&&` is recognized, but its execution path does not correctly implement shell-style short-circuiting. Do not rely on it for conditional commands.
- Output redirection for builtins currently creates the target file but leaves their `std::cout` output on the terminal. External command redirection works in the checked cases.
- Programmable argument completion currently aborts the shell when triggered in the checked case. Registering, displaying, and removing a completion specification work; command-name tab completion also works.
- `jobs` records and displays background processes, but does not detect completion or remove finished entries. There is no `fg`, `bg`, or `wait` builtin.
- Input redirection (`<`) has some supporting types but is not recognized by the active tokenizer. Pipelines (`|`) are not implemented.
- Syntax validation, multiline input, variable expansion, and exit status handling are incomplete. This is an interactive prototype rather than a full shell language.

## Tests

The GoogleTest suite covers tokenization, quoting, escaping, tilde expansion, and redirection parsing:

```sh
ctest --test-dir build --output-on-failure
```

## AI usage

I built the shell as a hands-on learning project: I researched the design, wrote the implementation, and worked through the changes myself. I did use AI to help find and understand documentation for system calls while learning how to use them. The CMake file was AI-assisted, and this README was drafted with AI after reviewing the code and Git diffs. No AI agent implemented the shell features.
