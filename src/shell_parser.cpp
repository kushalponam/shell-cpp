#include "shell_parser.h"
#include <iostream>

std::string extract_quoted_string(const std::string& input, size_t& i)
{
  std::string result;
  bool in_single_quote = false;
  bool in_double_quote = false;
  
  while (i < input.size()) 
  {
    char c = input[i];
    
    if (c == '\'' && !in_double_quote) 
    {
      in_single_quote = !in_single_quote;
    } 
    else if (c == '"' && !in_single_quote) 
    {
      in_double_quote = !in_double_quote;
    } 
    else if (c == '\\' && !in_single_quote && i + 1 < input.size()) 
    {
      // Handle escapes: in double quotes only for special chars, outside quotes for all
      bool should_escape = !in_double_quote || EscapedCharsInDoubleQuotes.contains(input[i + 1]);
      if (should_escape) 
      {
        result += input[++i];  // Skip backslash, add next char
      } 
      else 
      {
        result += c;  // Add backslash literally
      }
    } 
    else if (c == ' ' && !in_single_quote && !in_double_quote) 
    {
      break;  // End of this token
    } 
    else 
    {
      result += c;
    }
    
    i++;
  }
  
  return result;
}

void parse_input(const std::string& input, user_input& u_input)
{
  u_input.command.clear();
  u_input.args.clear();
  u_input.stdout_redirect_filename.clear();
  u_input.stderr_redirect_filename.clear();
  
  size_t i = 0;
  
  // Skip leading whitespace
  while (i < input.size() && input[i] == ' ') i++;
  
  // Extract command
  u_input.command = extract_quoted_string(input, i);
  
  // Skip whitespace after command
  while (i < input.size() && input[i] == ' ') i++;
  
  // Extract arguments
  while (i < input.size())
  {
    std::string arg = extract_quoted_string(input, i);
    if (!arg.empty())
    {
      u_input.args.push_back(arg);
    }
    // Skip whitespace between arguments
    while (i < input.size() && input[i] == ' ') i++;
  }

  if (u_input.has_arguments())
  {
    // check for redirection operator
    for (size_t j = 0; j < u_input.args.size(); j++)
    {
      if (u_input.args[j] == ">" || u_input.args[j] == "1>" || u_input.args[j] == ">>" || u_input.args[j] == "1>>")
      {
        if (j + 1 < u_input.args.size())
        {
          u_input.stdout_redirect_filename = u_input.args[j + 1];
          u_input.stdout_append = u_input.args[j] == ">>" || u_input.args[j] == "1>>";
          // Remove redirection operator and filename from args
          u_input.args.erase(u_input.args.begin() + j, u_input.args.begin() + j + 2);
        }
        else
        {
          std::cerr << "Error: No filename provided for redirection" << std::endl;
        }
      }
      else if (u_input.args[j] == "2>" || u_input.args[j] == "2>>")
      {
        if (j + 1 < u_input.args.size())
        {
          u_input.stderr_redirect_filename = u_input.args[j + 1];
          u_input.stderr_append = u_input.args[j] == "2>>";
          // Remove redirection operator and filename from args
          u_input.args.erase(u_input.args.begin() + j, u_input.args.begin() + j + 2);
        }
        else
        {
          std::cerr << "Error: No filename provided for redirection" << std::endl;
        }
      }
    }
  }
}
