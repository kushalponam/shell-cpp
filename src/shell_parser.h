#pragma once

#include "user_input.h"
#include <string>

// Extract a quoted/escaped string from input starting at position i
// Returns the processed string (without outer quotes) and advances i
std::string extract_quoted_string(const std::string& input, size_t& i);

// Parse input string into command and arguments
void parse_input(const std::string& input, user_input& u_input);

// Parse input that may contain pipelines
// Splits by '|' and calls parse_input on each segment
void parse_pipeline_input(const std::string& input, std::vector<user_input>& u_inputs);
