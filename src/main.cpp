#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Builtin command names
const std::string BUILTIN_ECHO = "echo";
const std::string BUILTIN_TYPE = "type";
const std::string BUILTIN_EXIT = "exit";
const std::string BUILTIN_PWD = "pwd";

// Check if a file has execute permissions
bool has_execute_permission(const fs::path& path) {
  fs::perms perms = fs::status(path).permissions();
  return ((perms & fs::perms::owner_exec) == fs::perms::owner_exec) ||
         ((perms & fs::perms::group_exec) == fs::perms::group_exec) ||
         ((perms & fs::perms::others_exec) == fs::perms::others_exec);
}

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

// Handle pwd builtin
void handle_pwd() {
  std::cout << fs::current_path().string() << std::endl;
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
    // Build candidate path
    std::string sep = "/";
    #ifndef __unix__
      sep = "\\";
    #endif
    
    std::string candidate = dir + sep + cmd;
    
    // Try with .exe extension on Windows if needed
    #ifndef __unix__
      if (!fs::exists(candidate)) {
        candidate += ".exe";
      }
    #endif
    
    // Check if file exists, is regular, and is executable
    if (fs::exists(candidate) && fs::is_regular_file(candidate) && has_execute_permission(candidate)) {
      full_path = candidate;
      return true;
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
  if (cmd == BUILTIN_ECHO || cmd == BUILTIN_TYPE || cmd == BUILTIN_EXIT || cmd == BUILTIN_PWD) {
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
    // Build command string
    // On Unix, use just the command name (system will search PATH)
    // On Windows, use the full path
    std::string exec_name = cmd;
    #ifndef __unix__
      exec_name = full_path;
    #endif
    
    std::string full_command = "\"" + exec_name + "\"";
    for (const auto& arg : args) {
      full_command += " " + arg;
    }
    system(full_command.c_str());
  } else {
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
    
    if (input == BUILTIN_EXIT) {
      break;
    }
  
    // Parse input into command and arguments
    std::string command;
    std::vector<std::string> args;
    parse_input(input, command, args);
    
    // Handle commands
    if (command == BUILTIN_ECHO) {
      handle_echo(args);
    } else if (command == BUILTIN_TYPE) {
      handle_type(args);
    } else if (command == BUILTIN_PWD) {
      handle_pwd();
    } else {
      // Try to execute as external command
      execute_external_command(command, args);
    }
  }

  return 0;
}


