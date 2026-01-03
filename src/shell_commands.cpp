#include "user_input.h"
#include "shell_commands.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <sstream>

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
