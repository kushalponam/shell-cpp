#pragma once

#include <string>
#include <vector>

// Handle echo builtin
void handle_echo(const std::vector<std::string>& args);

// Handle pwd builtin
void handle_pwd();

// Handle cd builtin
void handle_cd(const std::vector<std::string>& args);

// Handle type builtin
void handle_type(const std::vector<std::string>& args);

// Handle history builtin
void handle_history(const std::vector<std::string>& args);
