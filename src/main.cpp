#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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
    } else {
      // For now, all other commands are invalid
      std::cerr << command << ": command not found" << std::endl;
    }
  }

  return 0;
}

