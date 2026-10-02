#include "mysh/shell_state.hpp"

#include <cstdlib>
#include <system_error>

namespace mysh {

namespace fs = std::filesystem;

ShellState::ShellState() {
    if (const char* home = std::getenv("HOME"); home != nullptr) home_ = home;

    std::error_code ec;
    current_ = fs::current_path(ec);
    if (ec) current_ = "/";
}

Result<void> ShellState::change_directory(const fs::path& target) {
    std::error_code ec;
    fs::current_path(target, ec);  // chdir(2)
    if (ec) return make_error(ErrorCode::SystemError, target.string() + ": " + ec.message());

    fs::path now = fs::current_path(ec);
    if (ec) now = (target.is_absolute() ? target : current_ / target).lexically_normal();

    previous_ = std::move(current_);
    current_ = std::move(now);

    // Keep the environment consistent for child processes.
    ::setenv("OLDPWD", previous_.c_str(), 1);
    ::setenv("PWD", current_.c_str(), 1);
    return {};
}

}  // namespace mysh
