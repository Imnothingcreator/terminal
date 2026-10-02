#include "mysh/tokenizer.hpp"

#include <utility>

namespace mysh {

namespace {

constexpr bool is_space(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

enum class Mode { Normal, Single, Double };

}  // namespace

Result<std::vector<std::string>> tokenize(std::string_view line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_token = false;  // distinguishes an empty token ("") from no token
    Mode mode = Mode::Normal;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        switch (mode) {
            case Mode::Normal:
                if (is_space(c)) {
                    if (in_token) {
                        tokens.push_back(std::move(current));
                        current.clear();
                        in_token = false;
                    }
                } else if (c == '\'') {
                    mode = Mode::Single;
                    in_token = true;
                } else if (c == '"') {
                    mode = Mode::Double;
                    in_token = true;
                } else if (c == '\\') {
                    if (i + 1 >= line.size()) {
                        return make_error(ErrorCode::ParseError, "syntax error: trailing backslash");
                    }
                    current.push_back(line[++i]);
                    in_token = true;
                } else {
                    current.push_back(c);
                    in_token = true;
                }
                break;

            case Mode::Single:
                if (c == '\'') mode = Mode::Normal;
                else current.push_back(c);
                break;

            case Mode::Double:
                if (c == '"') {
                    mode = Mode::Normal;
                } else if (c == '\\' && i + 1 < line.size() &&
                           (line[i + 1] == '"' || line[i + 1] == '\\')) {
                    current.push_back(line[++i]);
                } else {
                    current.push_back(c);
                }
                break;
        }
    }

    if (mode != Mode::Normal) {
        return make_error(ErrorCode::ParseError,
                          mode == Mode::Single ? "syntax error: unterminated single quote"
                                               : "syntax error: unterminated double quote");
    }
    if (in_token) tokens.push_back(std::move(current));
    return tokens;
}

}  // namespace mysh
