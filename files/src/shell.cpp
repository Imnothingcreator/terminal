#include "mysh/shell.hpp"

#include <signal.h>
#include <unistd.h>

#include <iostream>

#include "mysh/parser.hpp"
#include "mysh/prompt.hpp"

namespace mysh {

namespace {

extern "C" void on_sigint(int) {}  // only needs to interrupt blocking read(2)

void install_signal_handlers() {
    struct sigaction sa {};
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // no SA_RESTART: Ctrl-C must interrupt read()
    ::sigaction(SIGINT, &sa, nullptr);

    // No job control: ignore Ctrl-\ and Ctrl-Z so the shell is never stopped
    // or killed by them (Ctrl-Z is inherited by children as ignored too).
    struct sigaction ign {};
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ::sigaction(SIGQUIT, &ign, nullptr);
    ::sigaction(SIGTSTP, &ign, nullptr);
}

}  // namespace

Shell::Shell()
    : builtins_(std::make_unique<BuiltinRegistry>()),
      executor_(std::make_unique<Executor>()),
      interactive_(::isatty(STDIN_FILENO) != 0) {
    install_signal_handlers();
}

int Shell::run() {
    while (state_.running()) {
        if (interactive_) std::cout << build_prompt(state_) << std::flush;

        auto [status, line] = reader_.read_line();
        if (status == ReadStatus::Eof) {
            if (interactive_) std::cout << "exit\n";
            break;
        }
        if (status == ReadStatus::Interrupted) {
            std::cout << '\n';
            continue;
        }
        execute_line(line);
    }
    return state_.last_status();
}

void Shell::execute_line(std::string_view line) {
    auto parsed = parse(line);
    if (!parsed) {
        std::cerr << "mysh: " << parsed.error().message << '\n';
        state_.set_last_status(exit_status_for(parsed.error().code));
        return;
    }
    if (!parsed->has_value()) return;  // empty line

    const Command& cmd = **parsed;

    if (const Builtin* builtin = builtins_->find(cmd.name); builtin != nullptr) {
        state_.set_last_status(builtin->handler(cmd, state_));
        return;
    }

    auto result = executor_->run(cmd);
    if (result) {
        state_.set_last_status(*result);
    } else {
        std::cerr << "mysh: " << result.error().message << '\n';
        state_.set_last_status(exit_status_for(result.error().code));
    }
}

}  // namespace mysh
