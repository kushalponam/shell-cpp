#include "user_input.h"
#include "shell_commands.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <readline/history.h>

namespace fs = std::filesystem;

void handle_echo(const std::vector<std::string>& args)
{
  for (size_t i = 0; i < args.size(); ++i)
  {
    std::cout << args[i] << (i < args.size() - 1 ? " " : "");
  }
  std::cout << std::endl;
}

void handle_pwd()
{
  std::cout << fs::current_path().string() << std::endl;
}

void handle_cd(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    std::cerr << "cd: wrong number of arguments" << std::endl;
    return;
  }
  
  std::string directory = args[0];
  const std::string original_arg = directory;  // Keep original for error messages
  
  // Handle ~ character (home directory)
  if (!directory.empty() && directory[0] == '~')
  {
    const char* home = std::getenv("HOME");
    if (home == nullptr)
    {
      std::cerr << "cd: HOME not set" << std::endl;
      return;
    }
    // Replace ~ with HOME directory
    directory = std::string(home) + directory.substr(1);
  }
  
  // Check if directory exists
  if (!fs::exists(directory) || !fs::is_directory(directory))
  {
    std::cerr << "cd: " << original_arg << ": No such file or directory" << std::endl;
    return;
  }
  
  // Change directory
  try
  {
    fs::current_path(directory);
  }
  catch (const std::exception&)
  {
    std::cerr << "cd: " << original_arg << ": No such file or directory" << std::endl;
  }
}

void handle_type(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    std::cerr << "type: invalid number of arguments" << std::endl;
    return;
  }
  
  const std::string& cmd = args[0];
  
  // Check if it's a builtin
  if (BuiltinCommands.contains(cmd))
  {
    std::cout << cmd << " is a shell builtin" << std::endl;
    return;
  }
  
  // Search for executable in PATH
  std::string full_path;
  // Forward declare find_in_path to use it here
  extern bool find_in_path(const std::string& cmd, std::string& full_path);
  
  if (find_in_path(cmd, full_path))
  {
    std::cout << cmd << " is " << full_path << std::endl;
  }
  else
  {
    std::cerr << cmd << ": not found" << std::endl;
  }
}

void handle_history(const std::vector<std::string>& args)
{
  if (args.size() == 0)
  {
    HIST_ENTRY **historyList = history_list();
    if (historyList)
    {
      for (int i = 0; historyList[i] != nullptr; i++)
      {
        std::cout << i + history_base << "  " << historyList[i]->line << std::endl;
      }
    }
  }
  else if (args.size() == 1)
  {
    try
    {
      int num_lines = std::stoi(args[0]);
      if (num_lines < 0)
      {
        std::cerr << "history: invalid number of lines: " << args[0] << std::endl;
        return;
      }
      
      HIST_ENTRY **historyList = history_list();
      if (historyList)
      {
        int total_entries = where_history() + 1;
        int start_index = std::max(0, total_entries - num_lines);
        for (int i = start_index; i < total_entries; i++)
        {
          std::cout << i + history_base << "  " << historyList[i]->line << std::endl;
        }
      }
    }
    catch (const std::invalid_argument&)
    {
      std::cerr << "history: invalid argument: " << args[0] << std::endl;
    }
    catch (const std::out_of_range&)
    {
      std::cerr << "history: argument out of range: " << args[0] << std::endl;
    }
  }
  else
  {
    std::cerr << "history: too many arguments" << std::endl;
  }
}
