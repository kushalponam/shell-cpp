#include <iostream>
#include <memory>
#include <string>

#include "user_input.h"
#include "shell_parser.h"
#include "shell_commands.h"
#include "shell_executor.h"

int main()
{
  // Flush after every std::cout / std::cerr

  std::string input;
  while (true)
  {
    std::cout << "$ ";
    if (!std::getline(std::cin, input) || input.empty())
    {
      continue;
    }
    
    if (input == BUILTIN_EXIT)
    {
      break;
    }
  
    // Parse input into command and arguments
    user_input u_input;
    parse_input(input, u_input);
    if (u_input.command.empty())
    {
      continue; // No command entered
    }
    
    // Handle output redirection if specified
    std::unique_ptr<stream_redirector> stdout_redir;
    if (u_input.has_stdout_redirect() && u_input.has_builtin_command())
    {
      try
      {
        std::ios_base::openmode mode = u_input.stdout_append ? 
                                       (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);

        stdout_redir = std::make_unique<stream_redirector>(std::cout, u_input.stdout_redirect_filename, mode);
      }
      catch (const std::exception& e)
      {
        std::cerr << e.what() << std::endl;
        continue;
      }
    }

    std::unique_ptr<stream_redirector> stderr_redir;
    if (u_input.has_stderr_redirect() && u_input.has_builtin_command())
    {
      try
      {
        std::ios_base::openmode mode = u_input.stderr_append ? 
                                       (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);

        stderr_redir = std::make_unique<stream_redirector>(std::cerr, u_input.stderr_redirect_filename, mode);
      }
      catch (const std::exception& e)
      {
        std::cerr << e.what() << std::endl;
        continue;
      }
    }
    
    // Handle commands
    if (u_input.command == BUILTIN_ECHO)
    {
      handle_echo(u_input.args);
    }
    else if (u_input.command == BUILTIN_TYPE)
    {
      handle_type(u_input.args);
    }
    else if (u_input.command == BUILTIN_PWD)
    {
      handle_pwd();
    }
    else if (u_input.command == BUILTIN_CD)
    {
      handle_cd(u_input.args);
    }
    else
    {
      // Try to execute as external command
      execute_external_command(u_input);
    }
  }

  return 0;
}


