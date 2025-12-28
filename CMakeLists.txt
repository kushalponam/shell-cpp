cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp LANGUAGES CXX)

find_package(readline QUIET)

# Fallback: use find_library if find_package didn't work
if(NOT readline_FOUND)
  find_library(READLINE_LIBRARY readline)
  find_path(READLINE_INCLUDE_DIR readline/readline.h)
  if(READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    set(readline_FOUND TRUE)
  endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
  src/main.cpp
  src/shell_parser.cpp
  src/shell_commands.cpp
  src/shell_executor.cpp
  src/user_input.cpp
)

add_executable(shell ${SOURCE_FILES})

# Link readline only on Linux
if(readline_FOUND)
  target_include_directories(shell PRIVATE ${READLINE_INCLUDE_DIR})
  target_link_libraries(shell PRIVATE ${READLINE_LIBRARY})
  message("Readline found and linked")
else()
  message("Readline not found, autocomplete disabled")
endif()
