cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
  src/main.cpp
  src/shell_parser.cpp
  src/shell_commands.cpp
  src/shell_executor.cpp
)

add_executable(shell ${SOURCE_FILES})

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#target_link_libraries(shell PRIVATE readline)
