#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
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
  bool stdout_append = false;
  bool stderr_append = false;

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

// RAII class to redirect any ostream to a file
class stream_redirector
{
  public:
    stream_redirector(std::ostream& stream, const std::string& filename, std::ios_base::openmode mode = std::ios::out | std::ios::trunc)
      : stream_(stream)
    {
      original_buf = stream.rdbuf();

      fs::path file_path(filename);
      if (file_path.has_parent_path())
      {
        fs::create_directories(file_path.parent_path());
      }

      file_stream.open(filename, mode);
      if (file_stream.is_open())
      {
        stream.rdbuf(file_stream.rdbuf());
      }
      else
      {
        throw std::runtime_error("Failed to open file for redirection");
      }
    }

    ~stream_redirector()
    {
      stream_.rdbuf(original_buf);
      if (file_stream.is_open())
      {
        file_stream.close();
      }
    }

  private:
    std::ostream& stream_;
    std::streambuf* original_buf;
    std::ofstream file_stream;
};
