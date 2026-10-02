#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "mysh/error.hpp"

namespace mysh {

// Splits a line into words.
//  - Unquoted whitespace separates tokens.
//  - 'single quotes' are fully literal.
//  - "double quotes" are literal except \" and \\ .
//  - Outside quotes, a backslash escapes the next character.
// Returns a ParseError for an unterminated quote or a trailing backslash.
[[nodiscard]] Result<std::vector<std::string>> tokenize(std::string_view line);

}  // namespace mysh
