#include "mysh/line_reader.hpp"

#include <unistd.h>

#include <array>
#include <cerrno>

namespace mysh {

ReadResult LineReader::read_line() {
    for (;;) {
        if (const auto pos = buffer_.find('\n'); pos != std::string::npos) {
            std::string line = buffer_.substr(0, pos);
            buffer_.erase(0, pos + 1);
            return {ReadStatus::Line, std::move(line)};
        }

        std::array<char, 4096> chunk{};
        const ssize_t n = ::read(STDIN_FILENO, chunk.data(), chunk.size());
        if (n > 0) {
            buffer_.append(chunk.data(), static_cast<std::size_t>(n));
        } else if (n == 0) {
            if (buffer_.empty()) return {ReadStatus::Eof, {}};
            // Final line without a trailing newline.
            std::string line = std::move(buffer_);
            buffer_.clear();
            return {ReadStatus::Line, std::move(line)};
        } else if (errno == EINTR) {
            buffer_.clear();  // the terminal discarded the partial line too
            return {ReadStatus::Interrupted, {}};
        } else {
            return {ReadStatus::Eof, {}};
        }
    }
}

}  // namespace mysh
