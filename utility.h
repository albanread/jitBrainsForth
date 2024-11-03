
#ifndef UTILITY_H
#define UTILITY_H
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cctype>


inline std::string trim(const std::string& str)
{
    const size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return ""; // no content

    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

inline bool is_number(const std::string& s)
{
    if (s.empty()) return false;
    size_t startIndex = 0;

    // Check for an optional leading minus
    if (s[0] == '-')
    {
        if (s.length() == 1) return false; // only '-' is not a valid number
        startIndex = 1;
    }

    for (size_t i = startIndex; i < s.length(); ++i)
    {
        if (!std::isdigit(s[i])) return false;
    }

    return true;
}


inline std::vector<std::string> split(const std::string& str)
{
    std::vector<std::string> result;
    std::istringstream iss(str);
    for (std::string word; iss >> word;)
        result.push_back(word);
    return result;
}

inline std::string to_lower(const std::string& s)
{
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

#endif //UTILITY_H
