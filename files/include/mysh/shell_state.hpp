#pragma once

#include <filesystem>
#include <string>

#include "mysh/error.hpp"

namespace mysh {

// All mutable shell state lives here; there are no globals.
class ShellState {
public:
    ShellState();

    [[nodiscard]] const std::filesystem::path& current_dir() const noexcept { return current_; }
    [[nodiscard]] const std::filesystem::path& previous_dir() const noexcept { return previous_; }
    [[nodiscard]] const std::string& home() const noexcept { return home_; }

    [[nodiscard]] int last_status() const noexcept { return last_status_; }
    void set_last_status(int status) noexcept { last_status_ = status; }

    [[nodiscard]] bool running() const noexcept { return running_; }
    void request_exit() noexcept { running_ = false; }

    // chdir + bookkeeping (previous dir, PWD/OLDPWD environment variables).
    [[nodiscard]] Result<void> change_directory(const std::filesystem::path& target);

private:
    std::filesystem::path current_;
    std::filesystem::path previous_;
    std::string home_;
    int last_status_ = 0;
    bool running_ = true;
};

}  // namespace mysh
