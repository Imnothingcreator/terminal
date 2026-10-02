#include "mysh/prompt.hpp"

namespace mysh {

std::string build_prompt(const ShellState& state) {
    std::string path = state.current_dir().string();
    const std::string& home = state.home();

    if (!home.empty() && home != "/") {
        if (path == home) {
            path = "~";
        } else if (path.size() > home.size() && path.starts_with(home) && path[home.size()] == '/') {
            path = "~" + path.substr(home.size());
        }
    }

    std::string prompt;
    prompt.reserve(kPromptName.size() + path.size() + 3);
    prompt += kPromptName;
    prompt += ':';
    prompt += path;
    prompt += "$ ";
    return prompt;
}

}  // namespace mysh
