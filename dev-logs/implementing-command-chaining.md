# implementing command chaining

To later implement the ampersand and (possibly) other command terminators up to
industry standard.

## todo list

1. create a new enum class to represent the semantic tag on tokens, and
   implement it while completely separating the tokenization logic and its enum
   from the parsing logic and its respective enum (the newly created one).
2. add a private vector to the parser class to store the index `i` at which a
   new command statement starts with token `tokens[i]`.
3. refactor so private variables of Parser like command, arguments,
   redirections, and so on, are vectors where each element at i represents the
   command, arguments vector, redirections vector, and so on, of the command
   statement number i. And getter functions like Parser::getCommand(),
   Parser::getArguments(), Parser::getRedirections(), and so on, take an int
   argument i that represents the command statement which the variable to be
   fetched belongs to.
4. refactor the Parser::parse function to a helper function that handles a
   single command statement, taking the index of the token to start parsing with
   in `tokens`, and returning the index at which the command statement is
   terminated. This new helper function should get invoked by the new parse
   function which loops over `tokens` and `tokenTypes` and appends to the
   private command statement vector of the Parser object.
5. refactor reading.cpp to use the new parser getter functions correctly.
6. refactor main.cpp's main to only include the loop and input gathering, while
   moving the execution logic of the shell to the ShellState object, which
   should internally handle multiple command statement execution. The builtins
   should also be refactored to use the current statement (which should be an
   iterator in the ShellState object passed to them) to use the parser getter
   functions correctly.
