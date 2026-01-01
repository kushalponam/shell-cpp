#include "Trie.h"
#include "user_input.h"
#include <map>
#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>
#include <climits>

extern std::map<std::string, std::string> Executables;
extern Trie *trie;

// Common matching logic for both platforms
std::vector<std::string> find_matching_commands(const std::string& prefix)
{
    std::set<std::string> unique_matches;
    for (const auto& cmd : BuiltinCommands)
    {
        if (!prefix.empty() && cmd.starts_with(prefix))  // Check if cmd starts with prefix
        {
            unique_matches.insert(cmd);
        }
    }

    for (const auto& [exe_name, exe_path] : Executables)
    {
        if (!prefix.empty() && exe_name.starts_with(prefix))  // Check if exe_name starts with prefix
        {
            unique_matches.insert(exe_name);
        }
    }
    
    return std::vector<std::string>(unique_matches.begin(), unique_matches.end());
}

static int tabCount = 0;
static std::string lastPrefix = "";
// Linux: readline completion callback
char** command_completion(const char* text, int start, int end)
{
    if (start != 0)
        return nullptr;

    std::string currentPrefix(text);
    if (currentPrefix != lastPrefix)
    {
        tabCount = 0; // Reset tab count if prefix has changed
        lastPrefix = currentPrefix;
    }

    std::vector<std::string> matches = find_matching_commands(std::string(text));

    if (matches.empty())
    {
        return nullptr;
    }

    if (matches.size() > 1)
    {
        tabCount++;
        if (tabCount == 1)
        {
            // First tab press: complete to longest common prefix. Can use Trie or simple binary search.
            std::string lcp = trie->getLongestCommonPrefix(currentPrefix);
            if (lcp != currentPrefix)
            {
                // Update the line with the new prefix
                rl_replace_line(lcp.c_str(), 0);
                rl_point = lcp.length();
            }
        }
        else if (tabCount >= 2)
        {
            // Second tab press: display all matches sorted alphabetically
            std::sort(matches.begin(), matches.end());
            
            std::cout << std::endl;
            for (size_t i = 0; i < matches.size(); i++)
            {
                std::cout << matches[i];
                if (i < matches.size() - 1)
                    std::cout << "  ";  // Two spaces between matches
            }
            std::cout << std::endl;
            rl_on_new_line();  // Tell readline we're on a new line
            rl_redisplay();    // Redisplay the prompt and input
            tabCount = 0;      // Reset for next prefix
        }
        return nullptr;
    }

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