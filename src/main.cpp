#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

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
    
    if (input == "exit")
    {
      break;
    }
  
    // Parse input into command and arguments
    std::istringstream iss(input);
    std::string command;
    std::vector<std::string> args;
    
    iss >> command;
    std::string arg;
    while (iss >> arg) {
      args.push_back(arg);
    }
    
    // Handle echo builtin
    if (command == "echo") 
    {
      for (size_t i = 0; i < args.size(); ++i) 
      {
        std::cout << args[i] << (i < args.size() - 1 ? " " : "");
      }
      std::cout << std::endl;
    }
    else if (command == "type")
    {
        if (args.size() != 1)
        {
          std::cerr << "type: invalid number of arguments" << std::endl;
        }
        else
        {
          const std::string& cmd = args[0];
          if (cmd == "echo" || cmd == "type" || cmd == "exit")
          {
            std::cout << cmd << " is a shell builtin" << std::endl;
          }
          else
          {
            // Search for executable in PATH
            const char* path_env = std::getenv("PATH");
            if (path_env == nullptr) {
              std::cerr << cmd << ": not found" << std::endl;
            } else {
              std::string path_str(path_env);
              std::istringstream path_stream(path_str);
              std::string dir;
              bool found = false;
              
              while (std::getline(path_stream, dir, ':')) {
                std::string full_path = dir + "/" + cmd;
                
                // Check if file exists and is a regular file
                if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
                  // Check if file has execute permissions
                  auto perms = fs::status(full_path).permissions();
                  // Check for owner execute permission (user execute)
                  if ((perms & fs::perm::owner_exec) != fs::perm::none) {
                    std::cout << cmd << " is " << full_path << std::endl;
                    found = true;
                    break;
                  }
                }
              }
              
              if (!found) {
                std::cerr << cmd << ": not found" << std::endl;
              }
            }
          }
        }
    } 
    else 
    {
      // For now, all other commands are invalid
      std::cerr << command << ": command not found" << std::endl;
    }
  }

  return 0;
}

