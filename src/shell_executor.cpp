#include "shell_executor.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

bool has_execute_permission(const fs::path& path)
{
    fs::perms perms = fs::status(path).permissions();
    return ((perms & fs::perms::owner_exec) == fs::perms::owner_exec) ||
           ((perms & fs::perms::group_exec) == fs::perms::group_exec) ||
           ((perms & fs::perms::others_exec) == fs::perms::others_exec);
}

std::map<std::string, std::string> get_all_executables_in_path()
{
    std::map<std::string, std::string> executables;
    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr)
    {
        return executables;
    }

    std::string path_str(path_env);
    std::istringstream path_stream(path_str);
    std::string dir;

    while (std::getline(path_stream, dir, ':'))
    {
        // Skip empty directory strings
        if (dir.empty())
        {
            continue;
        }

        // Iterate over files in directory
        try
        {
            for (const auto& entry :
                 fs::directory_iterator(dir, fs::directory_options::skip_permission_denied))
            {
                std::string filename = entry.path().filename().string();

                // Skip hidden files and files with extensions (quick checks before expensive fs
                // calls)
                if (filename[0] == '.' || filename.find('.') != std::string::npos)
                {
                    continue;
                }

                // Use symlink_status to avoid resolving symlinks, then check file type
                try
                {
                    fs::file_status status = fs::symlink_status(entry.path());
                    if (fs::is_regular_file(status) && has_execute_permission(entry.path()))
                    {
                        executables[filename] = entry.path().string();
                    }
                }
                catch (...)
                {
                    continue;
                }
            }
        }
        catch (...)
        {
            continue;
        }
    }

    return executables;
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
        if (fs::exists(candidate) && fs::is_regular_file(candidate) &&
            has_execute_permission(candidate))
        {
            full_path = candidate;
            return true;
        }
    }

    return false;
}

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

void execute_external_command(const user_input& u_input)
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
            full_command += " \"" + escaped_arg + "\"";
        }

        // Append redirection if specified
        if (u_input.has_stdout_redirect())
        {
            std::string operator_str = u_input.stdout_append ? " >> " : " > ";
            full_command += operator_str + "\"" + u_input.stdout_redirect_filename + "\"";
        }
        if (u_input.has_stderr_redirect())
        {
            std::string operator_str = u_input.stderr_append ? " 2>> " : " 2> ";
            full_command += operator_str + "\"" + u_input.stderr_redirect_filename + "\"";
        }

        system(full_command.c_str());
    }
    else
    {
        std::cerr << u_input.command << ": command not found" << std::endl;
    }
}
