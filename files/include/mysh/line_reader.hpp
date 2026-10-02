#pragma once

#include <string>

namespace mysh {

enum class ReadStatus { Line, Eof, Interrupted };

struct ReadResult {
    ReadStatus status;
    std::string line;
};

// Reads lines from stdin with read(2) so that a signal (Ctrl-C) can
// interrupt a blocked read and let the shell re-prompt.
class LineReader {
public:
    [[nodiscard]] ReadResult read_line();

private:
    std::string buffer_;
};

}  // namespace mysh
