#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <set>
namespace fs = std::filesystem;

// Builtin command names
const std::string BUILTIN_ECHO = "echo";
const std::string BUILTIN_TYPE = "type";
const std::string BUILTIN_EXIT = "exit";
const std::string BUILTIN_PWD = "pwd";
const std::string BUILTIN_CD = "cd";

const std::set<char> EscapedCharsInDoubleQuotes = {'$', '`', '"', '\\', '\n'};

struct user_input
{
  std::string command = "";
  std::vector<std::string> args = {};
  std::string stdout_redirect_filename = "";
  std::string stderr_redirect_filename = "";

  bool has_stdout_redirect() const
  {
    return !stdout_redirect_filename.empty();
  }

  bool has_stderr_redirect() const
  {
    return !stderr_redirect_filename.empty();
  }

  bool has_builtin_command() const
  {
    return command == BUILTIN_ECHO || command == BUILTIN_TYPE ||
           command == BUILTIN_EXIT || command == BUILTIN_PWD ||
           command == BUILTIN_CD;
  }

  bool has_arguments() const
  {
    return !args.empty();
  }
};

//RAII class to redirect stdout to a file
class stdout_redirector
{
  public:
    stdout_redirector(const std::string& filename)
    {
      original_buf = std::cout.rdbuf(); // Save original buffer

      fs::path file_path(filename);
      if (file_path.has_parent_path())
      {
        fs::create_directories(file_path.parent_path());
      }

      file_stream.open(filename, std::ios::out | std::ios::trunc);
      if (file_stream.is_open())
      {
        std::cout.rdbuf(file_stream.rdbuf()); // Redirect stdout to file
      }
      else
      {
        throw std::runtime_error("Failed to open file for redirection");
      }
    }

    ~stdout_redirector()
    {
      std::cout.rdbuf(original_buf); // Restore original buffer
      if (file_stream.is_open())
      {
        file_stream.close();
      }
    }

  private:
    std::streambuf* original_buf;
    std::ofstream file_stream;
};

//RAII class to redirect stderr to a file
class stderr_redirector
{
  public:
    stderr_redirector(const std::string& filename)
    {
      original_buf = std::cerr.rdbuf(); // Save original buffer

      fs::path file_path(filename);
      if (file_path.has_parent_path())
      {
        fs::create_directories(file_path.parent_path());
      }

      file_stream.open(filename, std::ios::out | std::ios::trunc);
      if (file_stream.is_open())
      {
        std::cerr.rdbuf(file_stream.rdbuf()); // Redirect stderr to file
      }
      else
      {
        throw std::runtime_error("Failed to open file for redirection");
      }
    }

    ~stderr_redirector()
    {
      std::cerr.rdbuf(original_buf); // Restore original buffer
      if (file_stream.is_open())
      {
        file_stream.close();
      }
    }

  private:
    std::streambuf* original_buf;
    std::ofstream file_stream;
};

// Check if a file has execute permissions
bool has_execute_permission(const fs::path& path)
{
  fs::perms perms = fs::status(path).permissions();
  return ((perms & fs::perms::owner_exec) == fs::perms::owner_exec) ||
         ((perms & fs::perms::group_exec) == fs::perms::group_exec) ||
         ((perms & fs::perms::others_exec) == fs::perms::others_exec);
}

// Extract a quoted/escaped string from input starting at position i
// Returns the processed string (without outer quotes) and advances i
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

// Parse input string into command and arguments
void parse_input(const std::string& input, user_input &u_input)
{
  u_input.command.clear();
  u_input.args.clear();
  u_input.stdout_redirect_filename.clear();
  
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
    for (size_t j = 0; j <u_input.args.size(); j++)
    {
      if (u_input.args[j] == ">" || u_input.args[j] == "1>")
      {
        if (j + 1 < u_input.args.size())
        {
          u_input.stdout_redirect_filename = u_input.args[j + 1];
          // Remove redirection operator and filename from args
          u_input.args.erase(u_input.args.begin() + j, u_input.args.begin() + j + 2);
        }
        else
        {
          std::cerr << "Error: No filename provided for redirection" << std::endl;
        }
        //break; // Only handle first redirection
      }
      else if (u_input.args[j] == "2>")
      {
        if (j + 1 < u_input.args.size())
        {
          u_input.stderr_redirect_filename = u_input.args[j + 1];
          // Remove redirection operator and filename from args
          u_input.args.erase(u_input.args.begin() + j, u_input.args.begin() + j + 2);
        }
        else
        {
          std::cerr << "Error: No filename provided for redirection" << std::endl;
        }
        //break; // Only handle first redirection
      }
    }
  }
}

// Handle echo builtin
void handle_echo(const std::vector<std::string>& args)
{
  for (size_t i = 0; i < args.size(); ++i)
  {
    std::cout << args[i] << (i < args.size() - 1 ? " " : "");
  }
  std::cout << std::endl;
}

// Handle pwd builtin
void handle_pwd()
{
  std::cout << fs::current_path().string() << std::endl;
}

// Handle cd builtin
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

bool find_in_path(const std::string& cmd, std::string& full_path)
{
  const char* path_env = std::getenv("PATH");
  if (path_env == nullptr)
  {
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
  
  while (std::getline(path_stream, dir, separator))
  {
    // Build candidate path
    std::string sep = "/";
    #ifndef __unix__
      sep = "\\";
    #endif
    
    std::string candidate = dir + sep + cmd;
    
    // Try with .exe extension on Windows if needed
    #ifndef __unix__
      if (!fs::exists(candidate))
      {
        candidate += ".exe";
      }
    #endif
    
    // Check if file exists, is regular, and is executable
    if (fs::exists(candidate) && fs::is_regular_file(candidate) && has_execute_permission(candidate))
    {
      full_path = candidate;
      return true;
    }
  }
  
  return false;
}

// Handle type builtin
void handle_type(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    std::cerr << "type: invalid number of arguments" << std::endl;
    return;
  }
  
  const std::string& cmd = args[0];
  
  // Check if it's a builtin
  if (cmd == BUILTIN_ECHO || cmd == BUILTIN_TYPE || cmd == BUILTIN_EXIT || cmd == BUILTIN_PWD || cmd == BUILTIN_CD)
  {
    std::cout << cmd << " is a shell builtin" << std::endl;
    return;
  }
  
  // Search for executable in PATH
  std::string full_path;
  if (find_in_path(cmd, full_path))
  {
    std::cout << cmd << " is " << full_path << std::endl;
  }
  else
  {
    std::cerr << cmd << ": not found" << std::endl;
  }
}

// Escape quotes in a string for passing to system()
// Returns the escaped string and sets needs_quoting if escaping occurred or string has special chars
std::string escape_for_shell(const std::string& str, bool& needs_quoting)
{
  needs_quoting = false;
  std::string result;
  for (char c : str)
  {
    if (c == '"')
    {
      result += '\\';
      needs_quoting = true;
    }
    else if (c == ' ' || c == '\'')
    {
      // Single quotes and spaces need quoting (wrap in double quotes)
      needs_quoting = true;
    }
    result += c;
  }
  return result;
}

// Execute external command using system()
void execute_external_command(const user_input &u_input)
{
  std::string full_path;
  if (find_in_path(u_input.command, full_path))
  {
    // Build command string - quote only if necessary
    bool needs_quoting;
    std::string escaped_cmd = escape_for_shell(u_input.command, needs_quoting);
    std::string full_command = needs_quoting ? "\"" + escaped_cmd + "\"" : escaped_cmd;
    
    for (const auto& arg : u_input.args)
    {
      std::string escaped_arg = escape_for_shell(arg, needs_quoting);
      full_command +=  " \"" + escaped_arg + "\"";
    }
    
    // Append redirection if specified
    if (u_input.has_stdout_redirect())
    {
      full_command += " > \"" + u_input.stdout_redirect_filename + "\"";
    }
    if (u_input.has_stderr_redirect())
    {
      full_command += " 2> \"" + u_input.stderr_redirect_filename + "\"";
    }
    
    system(full_command.c_str());
  }
  else
  {
    std::cerr << u_input.command << ": command not found" << std::endl;
  }
}

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

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
    std::unique_ptr<stdout_redirector> redirector;
    if (u_input.has_stdout_redirect() && u_input.has_builtin_command())
    {
      // Only redirect for builtins, not external commands
      // External commands will handle redirection via shell
      try
      {
        redirector = std::make_unique<stdout_redirector>(u_input.stdout_redirect_filename);
      }
      catch (const std::exception& e)
      {
        std::cerr << e.what() << std::endl;
        continue;
      }
    }

    std::unique_ptr<stderr_redirector> err_redirector;
    if (u_input.has_stderr_redirect() && u_input.has_builtin_command())
    {
      // Only redirect for builtins, not external commands
      // External commands will handle redirection via shell
      try
      {
        err_redirector = std::make_unique<stderr_redirector>(u_input.stderr_redirect_filename);
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


