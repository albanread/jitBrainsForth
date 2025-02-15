
#ifndef ASSEMBLERUTILS_H
#define ASSEMBLERUTILS_H
#include <cstdint>
#include <iosfwd>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace AssemblerUtils {

    inline std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos)
                   ? ""
                   : str.substr(first, last - first + 1);
    }

    inline std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }

    inline uint64_t parseImmediateValue(const std::string& immediate) {
        try {
            if (immediate.starts_with("0x") || immediate.starts_with("$")) {
                return std::stoull(immediate.substr(immediate.starts_with("0x") ? 2 : 1), nullptr, 16);
            } else {
                return std::stoull(immediate, nullptr, 10);
            }
        } catch (...) {
            throw std::runtime_error("Invalid immediate value: " + immediate);
        }
    }
}
#endif //ASSEMBLERUTILS_H
