#pragma once

#include <string>
#include <string_view>

#include "mysh/shell_state.hpp"

namespace mysh {

inline constexpr std::string_view kPromptName = "mysh";

// Renders e.g. "mysh:~/projects$ " ($HOME is abbreviated as "~").
[[nodiscard]] std::string build_prompt(const ShellState& state);

}  // namespace mysh
