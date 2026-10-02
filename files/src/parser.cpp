#include "mysh/parser.hpp"

#include <utility>

#include "mysh/tokenizer.hpp"

namespace mysh {

Result<std::optional<Command>> parse(std::string_view line) {
    auto tokens = tokenize(line);
    if (!tokens) return std::unexpected(std::move(tokens.error()));
    if (tokens->empty()) return std::optional<Command>{};

    Command cmd;
    cmd.name = std::move(tokens->front());
    cmd.args.assign(std::make_move_iterator(tokens->begin() + 1),
                    std::make_move_iterator(tokens->end()));
    return std::optional<Command>{std::move(cmd)};
}

}  // namespace mysh
