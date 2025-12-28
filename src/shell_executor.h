#pragma once

#include "user_input.h"
#include <map>
#include <string>

// Check if a file has execute permissions
bool has_execute_permission(const std::filesystem::path& path);

// Find command in PATH
bool find_in_path(const std::string& cmd, std::string& full_path);

std::map<std::string, std::string> get_all_executables_in_path();

// Escape quotes in a string for passing to system()
std::string escape_for_shell(const std::string& str, bool& needs_quoting);

// Execute external command using system()
void execute_external_command(const user_input& u_input);
