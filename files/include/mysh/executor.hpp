#pragma once

#include "mysh/error.hpp"
#include "mysh/parser.hpp"

namespace mysh {

// Runs external programs (fork / execvp / waitpid).
class Executor {
public:
    // On success returns the exit status (128+N if killed by signal N).
    // Errors: CommandNotFound (127), NotExecutable (126), SystemError (1).
    [[nodiscard]] Result<int> run(const Command& cmd);
};

}  // namespace mysh
