#pragma once

#include <expected>
#include <string>
#include <utility>

namespace mysh {

enum class ErrorCode {
    ParseError,
    InvalidArgument,
    CommandNotFound,
    NotExecutable,
    SystemError,
};

struct ShellError {
    ErrorCode code;
    std::string message;
};

template <typename T>
using Result = std::expected<T, ShellError>;

[[nodiscard]] inline std::unexpected<ShellError> make_error(ErrorCode code, std::string message) {
    return std::unexpected<ShellError>{ShellError{code, std::move(message)}};
}

// Conventional shell exit statuses for each error category.
[[nodiscard]] constexpr int exit_status_for(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::CommandNotFound: return 127;
        case ErrorCode::NotExecutable:   return 126;
        case ErrorCode::ParseError:      return 2;
        case ErrorCode::InvalidArgument: return 2;
        case ErrorCode::SystemError:     return 1;
    }
    return 1;
}

}  // namespace mysh
