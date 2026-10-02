#pragma once

#include <memory>
#include <string_view>

#include "mysh/builtins.hpp"
#include "mysh/executor.hpp"
#include "mysh/line_reader.hpp"
#include "mysh/shell_state.hpp"

namespace mysh {

class Shell {
public:
    Shell();

    // Runs the REPL; returns the process exit status.
    [[nodiscard]] int run();

private:
    void execute_line(std::string_view line);

    ShellState state_;
    LineReader reader_;
    std::unique_ptr<BuiltinRegistry> builtins_;
    std::unique_ptr<Executor> executor_;
    bool interactive_;
};

}  // namespace mysh
