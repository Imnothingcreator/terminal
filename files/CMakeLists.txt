cmake_minimum_required(VERSION 3.20)
project(mysh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mysh
    src/main.cpp
    src/shell.cpp
    src/line_reader.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/shell_state.cpp
    src/prompt.cpp
    src/builtins.cpp
    src/executor.cpp
)

target_include_directories(mysh PRIVATE include)
target_compile_options(mysh PRIVATE -Wall -Wextra -Wpedantic)
