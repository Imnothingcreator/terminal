#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

#include "mysh/parser.hpp"
#include "mysh/shell_state.hpp"

namespace mysh {

// A builtin returns its exit status.
using BuiltinHandler = std::function<int(const Command&, ShellState&)>;

struct Builtin {
    std::string usage;
    std::string description;
    BuiltinHandler handler;
};

class BuiltinRegistry {
public:
    BuiltinRegistry();

    // The help handler captures `this`, so the registry must stay put.
    BuiltinRegistry(const BuiltinRegistry&) = delete;
    BuiltinRegistry& operator=(const BuiltinRegistry&) = delete;
    BuiltinRegistry(BuiltinRegistry&&) = delete;
    BuiltinRegistry& operator=(BuiltinRegistry&&) = delete;

    [[nodiscard]] const Builtin* find(std::string_view name) const;

private:
    void add(std::string name, std::string usage, std::string description, BuiltinHandler handler);
    [[nodiscard]] int help(const Command& cmd) const;

    std::map<std::string, Builtin, std::less<>> builtins_;
};

}  // namespace mysh
