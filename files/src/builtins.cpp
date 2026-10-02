#include "mysh/builtins.hpp"

#include <charconv>
#include <iostream>
#include <system_error>

namespace mysh {

namespace {

std::string expand_tilde(const std::string& arg, const std::string& home) {
    if (home.empty()) return arg;
    if (arg == "~") return home;
    if (arg.starts_with("~/")) return home + arg.substr(1);
    return arg;
}

int builtin_cd(const Command& cmd, ShellState& state) {
    if (cmd.args.size() > 1) {
        std::cerr << "cd: too many arguments\n";
        return 1;
    }

    std::string target;
    bool print_dir = false;

    if (cmd.args.empty()) {
        if (state.home().empty()) {
            std::cerr << "cd: HOME not set\n";
            return 1;
        }
        target = state.home();
    } else if (cmd.args.front() == "-") {
        if (state.previous_dir().empty()) {
            std::cerr << "cd: OLDPWD not set\n";
            return 1;
        }
        target = state.previous_dir().string();
        print_dir = true;
    } else {
        target = expand_tilde(cmd.args.front(), state.home());
    }

    if (auto result = state.change_directory(target); !result) {
        std::cerr << "cd: " << result.error().message << '\n';
        return 1;
    }
    if (print_dir) std::cout << state.current_dir().string() << '\n';
    return 0;
}

int builtin_exit(const Command& cmd, ShellState& state) {
    if (cmd.args.size() > 1) {
        std::cerr << "exit: too many arguments\n";
        return 1;
    }

    int status = state.last_status();
    if (!cmd.args.empty()) {
        const std::string& arg = cmd.args.front();
        int value = 0;
        const auto [ptr, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), value);
        if (ec != std::errc{} || ptr != arg.data() + arg.size()) {
            std::cerr << "exit: " << arg << ": numeric argument required\n";
            return 2;  // stay in the shell
        }
        status = value & 0xFF;
    }

    state.request_exit();
    return status;
}

int builtin_clear(const Command&, ShellState&) {
    // Cursor home, clear screen, clear scrollback.
    std::cout << "\033[H\033[2J\033[3J" << std::flush;
    return 0;
}

}  // namespace

BuiltinRegistry::BuiltinRegistry() {
    add("help", "help [command]", "Show available builtins or help for one.",
        [this](const Command& cmd, ShellState&) { return help(cmd); });
    add("exit", "exit [status]", "Exit the shell, optionally with a status code.", builtin_exit);
    add("clear", "clear", "Clear the terminal screen.", builtin_clear);
    add("cd", "cd [dir | -]", "Change directory (no argument: $HOME, '-': previous).", builtin_cd);
}

void BuiltinRegistry::add(std::string name, std::string usage, std::string description,
                          BuiltinHandler handler) {
    builtins_.emplace(std::move(name),
                      Builtin{std::move(usage), std::move(description), std::move(handler)});
}

const Builtin* BuiltinRegistry::find(std::string_view name) const {
    const auto it = builtins_.find(name);
    return it == builtins_.end() ? nullptr : &it->second;
}

int BuiltinRegistry::help(const Command& cmd) const {
    if (cmd.args.size() > 1) {
        std::cerr << "help: too many arguments\n";
        return 1;
    }

    if (cmd.args.size() == 1) {
        const Builtin* b = find(cmd.args.front());
        if (b == nullptr) {
            std::cerr << "help: no help for '" << cmd.args.front() << "'\n";
            return 1;
        }
        std::cout << "Usage: " << b->usage << "\n  " << b->description << '\n';
        return 0;
    }

    std::cout << "mysh builtins:\n";
    for (const auto& [name, b] : builtins_) {
        std::cout << "  " << b.usage;
        for (std::size_t i = b.usage.size(); i < 16; ++i) std::cout << ' ';
        std::cout << b.description << '\n';
    }
    std::cout << "Any other command is run as an external program.\n"
                 "Quoting: 'literal', \"with \\\" escapes\", and backslash escapes are supported.\n";
    return 0;
}

}  // namespace mysh
