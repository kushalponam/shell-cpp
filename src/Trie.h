#pragma once

#include <unordered_map>
#include <string>
#include <functional>

class Trie
{
    public:
        Trie()
        {
            root = new Node();
            root->isLeaf = false;
        }
        ~Trie()
        {
            // Properly delete all nodes to avoid memory leaks
            std::function<void(Node*)> deleteNodes = [&](Node* node) {
                if (!node) return;
                for (auto& [key, child] : node->children) {
                    deleteNodes(child);
                }
                delete node;
            };
            deleteNodes(root);
        }

        void insert(const std::string& word)
        {
            Node* current = root;
            for (char c : word)
            {
                if (current->children.find(c) == current->children.end())
                {
                    current->children[c] = new Node();
                    current->children[c]->isLeaf = false;
                }
                current = current->children[c];
            }
            current->isLeaf = true;
        }

        bool search(const std::string& word)
        {
            Node* current = root;
            for (char c : word)
            {
                if (current->children.find(c) == current->children.end())
                {
                    return false;
                }
                current = current->children[c];
            }
            return current->isLeaf;
        }

        std::string getLongestCommonPrefix(const std::string& input)
        {
            // Returns the longest common prefix of 'input' that exists in the Trie
            // ex: if input is "xyz_" and Trie contains "xyz_bar", "xyz_bar_baz", it returns "xyz_bar"
            Node* current = root;
    
            // Navigate to the node representing the input prefix
            for (char c : input)
            {
                if (current->children.find(c) == current->children.end())
                {
                    return input;  // Prefix not found, return as-is
                }
                current = current->children[c];
            }
            
            std::string result = input;
            
            // Follow single-child path to find the LCP
            while (current->children.size() == 1 && !current->isLeaf)
            {
                auto [ch, child] = *current->children.begin();
                result += ch;
                current = child;
            }
            
            return result;
        }
    private:
        struct Node{
            std::unordered_map<char, Node*> children;
            bool isLeaf;
        };

        Node* root;
};