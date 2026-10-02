#include "mysh/executor.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "mysh/unique_fd.hpp"

namespace mysh {

namespace {

struct Pipe {
    UniqueFd read_end;
    UniqueFd write_end;
};

// A pipe whose ends close automatically on a successful exec.
Result<Pipe> make_cloexec_pipe() {
    int fds[2];
    if (::pipe(fds) != 0) {
        return make_error(ErrorCode::SystemError, std::string("pipe: ") + std::strerror(errno));
    }
    Pipe p{UniqueFd{fds[0]}, UniqueFd{fds[1]}};
    for (const int fd : fds) {
        if (::fcntl(fd, F_SETFD, FD_CLOEXEC) != 0) {
            return make_error(ErrorCode::SystemError,
                              std::string("fcntl: ") + std::strerror(errno));
        }
    }
    return p;
}

void reset_child_signals() {
    struct sigaction sa {};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    ::sigaction(SIGINT, &sa, nullptr);
    ::sigaction(SIGQUIT, &sa, nullptr);
}

}  // namespace

Result<int> Executor::run(const Command& cmd) {
    // Build argv before forking: only async-signal-safe calls after fork().
    std::vector<std::string> storage;
    storage.reserve(cmd.args.size() + 1);
    storage.push_back(cmd.name);
    storage.insert(storage.end(), cmd.args.begin(), cmd.args.end());

    std::vector<char*> argv;
    argv.reserve(storage.size() + 1);
    for (auto& s : storage) argv.push_back(s.data());
    argv.push_back(nullptr);

    auto pipe = make_cloexec_pipe();
    if (!pipe) return std::unexpected(std::move(pipe.error()));

    std::cout.flush();
    std::cerr.flush();

    const pid_t pid = ::fork();
    if (pid < 0) {
        return make_error(ErrorCode::SystemError, std::string("fork: ") + std::strerror(errno));
    }

    if (pid == 0) {
        reset_child_signals();
        ::execvp(argv[0], argv.data());
        // exec failed: report errno to the parent through the pipe.
        const int err = errno;
        [[maybe_unused]] const ssize_t ignored = ::write(pipe->write_end.get(), &err, sizeof err);
        ::_exit(127);
    }

    // Parent.
    pipe->write_end.reset();

    int child_errno = 0;
    ssize_t n = 0;
    do {
        n = ::read(pipe->read_end.get(), &child_errno, sizeof child_errno);
    } while (n < 0 && errno == EINTR);

    int status = 0;
    while (::waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return make_error(ErrorCode::SystemError,
                              std::string("waitpid: ") + std::strerror(errno));
        }
    }

    if (n == static_cast<ssize_t>(sizeof child_errno)) {
        // The program never started.
        switch (child_errno) {
            case ENOENT:
                return make_error(ErrorCode::CommandNotFound, cmd.name + ": command not found");
            case EACCES:
                return make_error(ErrorCode::NotExecutable, cmd.name + ": permission denied");
            default:
                return make_error(ErrorCode::NotExecutable,
                                  cmd.name + ": " + std::strerror(child_errno));
        }
    }

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) {
        const int sig = WTERMSIG(status);
        if (sig == SIGINT) std::cerr << '\n';
        else std::cerr << ::strsignal(sig) << '\n';
        return 128 + sig;
    }
    return 1;
}

}  // namespace mysh
