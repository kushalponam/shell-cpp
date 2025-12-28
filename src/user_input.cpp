#include "user_input.h"
#include <readline/readline.h>
#include <readline/history.h>


// Common matching logic for both platforms
std::vector<std::string> find_matching_commands(const std::string& prefix)
{
    std::vector<std::string> matches;
    for (const auto& cmd : BuiltinCommands)
    {
        if (!prefix.empty() && cmd.starts_with(prefix))  // Check if cmd starts with prefix
        {
            matches.push_back(cmd);
        }
    }
    return matches;
}

// Linux: readline completion callback
char** command_completion(const char* text, int start, int end)
{
    if (start != 0)
        return nullptr;

    std::vector<std::string> matches = find_matching_commands(std::string(text));

    if (matches.empty())
        return nullptr;

    char** result = (char**)malloc((matches.size() + 1) * sizeof(char*));
    for (size_t i = 0; i < matches.size(); i++)
    {
        result[i] = (char*)malloc(matches[i].length() + 1);
        strcpy(result[i], matches[i].c_str());
    }
    result[matches.size()] = nullptr;

    return result;
}

std::string GetUserInput()
{
    // Linux: use readline with completion callback
    rl_attempted_completion_function = command_completion;

    char* line = readline("$ ");

    if (line == nullptr) return "";

    std::string input(line);

    if (!input.empty())
        add_history(line);

    free(line);
    return input;
}