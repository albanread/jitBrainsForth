#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>


#include "jitGenerator.h"
#include "ForthDictionary.h"


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


inline void compileWord(const std::string& wordName, const std::string& compileText)
{
    printf("\nCompiler: compiling word: %s\n", wordName.c_str());

    bool bailout = false;

    JitContext& jc = JitContext::getInstance();
    jc.resetContext();

    // compiling into the new word
    JitGenerator::genPrologue();

    const auto words = split(compileText);

    size_t i = 0;

    while (i < words.size())
    {
        const auto& word = to_lower(words[i]);

        ForthWord* fword = d.findWord(word.c_str());
        if (fword->generatorFunc != nullptr)
        {
            fword->generatorFunc();
            i++;
        }
        else if (fword->compiledFunc != nullptr)
        {
            // gen_call_word(context, execFunc);
            ++i;
            printf("Compiler ... exec word %s\n", word.c_str());
        }
        else if (is_number(word))
        {
            const uint64_t number = std::stoll(word.c_str());
            // Generate code to push the number onto the stack
            jc.uint64_A = number;
            JitGenerator::genPushLong();
            ++i;
            printf("Compiler ... number word %s\n", word.c_str());
        }
        else
        {
            std::cout << "Error: Unknown or uncompilable word: " << word << std::endl;
            bailout = true;
            // we may as well stop processing other words now
            break;
        }
    }

    if(bailout)
    {
        // clean up asmjit

        printf("Compiler: Error building : %s\n", wordName.c_str());
        return;
    }
    // check if word in dictionary
    if ( d.findWord(wordName.c_str()) != nullptr )
    {
        printf("Compiler: word already exists: %s\n", wordName.c_str());
        bailout = true;
        // we may as well stop processing other words now
        jc.resetContext();
        return;
    }


    // finalize word here
    JitGenerator::genEpilogue();
    const ForthFunction f = JitGenerator::end();
    d.addWord(wordName.c_str(), f, nullptr);
    printf("Compiler: compiled word: %s\n", wordName.c_str());
    // print code size
    std::cout << "Code size: " << jc.code.codeSize() << std::endl;
}


#endif //INTERPRETER_H
