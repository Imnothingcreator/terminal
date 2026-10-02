#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "mysh/error.hpp"

namespace mysh {

struct Command {
    std::string name;
    std::vector<std::string> args;
};

// Empty or whitespace-only input yields std::nullopt (not an error).
[[nodiscard]] Result<std::optional<Command>> parse(std::string_view line);

}  // namespace mysh
