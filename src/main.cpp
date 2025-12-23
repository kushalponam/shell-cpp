#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Parse input string into command and arguments
void parse_input(const std::string& input, std::string& command, std::vector<std::string>& args) {
  std::istringstream iss(input);
  command.clear();
  args.clear();
  
  iss >> command;
  std::string arg;
  while (iss >> arg) {
    args.push_back(arg);
  }
}

// Handle echo builtin
void handle_echo(const std::vector<std::string>& args) {
  for (size_t i = 0; i < args.size(); ++i) {
    std::cout << args[i] << (i < args.size() - 1 ? " " : "");
  }
  std::cout << std::endl;
}

bool find_in_path(const std::string& cmd, std::string& full_path) {
  const char* path_env = std::getenv("PATH");
  if (path_env == nullptr) {
    return false;
  }
  
  std::string path_str(path_env);
  std::istringstream path_stream(path_str);
  std::string dir;

  // On Windows, PATH is separated by ';', on Unix by ':'
  char separator = ';';
  #ifdef __unix__
    separator = ':';
  #endif
  
  while (std::getline(path_stream, dir, separator)) {
    std::string candidate = dir;
    #ifdef __unix__
      candidate += "/" + cmd;
    #else
      candidate += "\\" + cmd;
      // Try with .exe extension on Windows if the file without extension doesn't exist
      if (!fs::exists(candidate)) {
        candidate += ".exe";
      }
    #endif
    
    if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
      fs::perms filePerms = fs::status(candidate).permissions();
      if (((filePerms & fs::perms::owner_exec) == fs::perms::owner_exec) ||
          ((filePerms & fs::perms::group_exec) == fs::perms::group_exec) ||
          ((filePerms & fs::perms::others_exec) == fs::perms::others_exec)) {
        full_path = candidate;
        return true;
      }
    }
  }
  
  return false;
}

// Handle type builtin
void handle_type(const std::vector<std::string>& args) {
  if (args.size() != 1) {
    std::cerr << "type: invalid number of arguments" << std::endl;
    return;
  }
  
  const std::string& cmd = args[0];
  
  // Check if it's a builtin
  if (cmd == "echo" || cmd == "type" || cmd == "exit") {
    std::cout << cmd << " is a shell builtin" << std::endl;
    return;
  }
  
  // Search for executable in PATH
  std::string full_path;
  if (find_in_path(cmd, full_path)) {
    std::cout << cmd << " is " << full_path << std::endl;
  } else {
    std::cerr << cmd << ": not found" << std::endl;
  }
}

// Execute external command using system()
void execute_external_command(const std::string& cmd, const std::vector<std::string>& args) {
  
  std::string full_path;
  if (find_in_path(cmd, full_path)) {
    // Wrap full_path in quotes to handle spaces in paths
    std::string full_command = "\"" + full_path + "\"";
    for (const auto& arg : args) {
      full_command += " " + arg;
    }
    system(full_command.c_str());
  } 
  else 
  {
    std::cerr << cmd << ": command not found" << std::endl;
  }
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string input;
  while (true) {
    std::cout << "$ ";
    if (!std::getline(std::cin, input) || input.empty()) {
      continue;
    }
    
    if (input == "exit") {
      break;
    }
  
    // Parse input into command and arguments
    std::string command;
    std::vector<std::string> args;
    parse_input(input, command, args);
    
    // Handle commands
    if (command == "echo") {
      handle_echo(args);
    } else if (command == "type") {
      handle_type(args);
    } else {
      // Try to execute as external command
      execute_external_command(command, args);
    }
  }

  return 0;
}


