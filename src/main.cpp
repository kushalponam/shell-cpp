#include <iostream>
#include <memory>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "Trie.h"
#include "user_input.h"
#include "shell_parser.h"
#include "shell_commands.h"
#include "shell_executor.h"
#include <readline/history.h>

bool initialized_executables = false;
std::map<std::string, std::string> Executables;
Trie *trie;


void ExecuteInputCommand(const user_input& u_input)
{
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
  else if (u_input.command == BUILTIN_HISTORY)
  {
    handle_history(u_input.args);
  }
  else
  {
    // Try to execute as external command
    execute_external_command(u_input);
  }
}

int main()
{
  // Read from HISTFILE if set (only if it's a regular file)
  const char* histfile = std::getenv("HISTFILE");
  if (histfile && std::filesystem::exists(histfile) && std::filesystem::is_regular_file(histfile))
  {
    read_history(histfile);
    // Silently ignore errors during startup history loading
  }

  std::string input;
  while (true)
  {
    if (!initialized_executables)
    {
      Executables = get_all_executables_in_path();
      trie = new Trie();
      for (const auto& [exe_name, exe_path] : Executables)
      {
        trie->insert(exe_name);
      }

      initialized_executables = true;
    }

    // Get user input
    input = GetUserInput();

    if (input.empty())
    {
      continue;
    }
    
    if (input == BUILTIN_EXIT)
    {
      break;
    }
  
    // Parse input into command and arguments
    std::vector<user_input> u_inputs;
    parse_pipeline_input(input, u_inputs);
    if (u_inputs.empty())
    {
      continue; // No command entered
    }

    if (u_inputs.size() == 1)
    {
      // Single command, no pipeline
      ExecuteInputCommand(u_inputs[0]);
      continue;
    }
    
    int pipe_fds[u_inputs.size() - 1][2];
    for (size_t i = 0; i < u_inputs.size() - 1; i++)
    {
      if (pipe(pipe_fds[i]) == -1)
      {
        std::cerr << "Error creating pipe" << std::endl;
        return 1;
      }
    }

    for (size_t i = 0; i < u_inputs.size(); i++)
    {
      pid_t pid = fork();
      if (pid == 0)
      {
        // Child process
        // Set up pipes
        if (i > 0) dup2(pipe_fds[i - 1][0], STDIN_FILENO);
        
        if (i < u_inputs.size() - 1) dup2(pipe_fds[i][1], STDOUT_FILENO);
        
        // Close all pipe fds in child
        for (size_t j = 0; j < u_inputs.size() - 1; j++)
        {
          close(pipe_fds[j][0]);
          close(pipe_fds[j][1]);
        }

        ExecuteInputCommand(u_inputs[i]);
        exit(0);
      }
      else if (pid < 0)
      {
        std::cerr << "Error forking process" << std::endl;
        return 1;
      } 
    }

    for (size_t i = 0; i < u_inputs.size() - 1; i++)
    {
      // Close all pipe fds in parent
      close(pipe_fds[i][0]);
      close(pipe_fds[i][1]);
    }
    // Wait for all child processes
    for (size_t i = 0; i < u_inputs.size(); i++)
    {
      waitpid(-1, nullptr, 0);
    }
  }
  
  if (trie)
  {
    delete trie;
  }

  return 0;
}


