#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#include "utility.h"
#include "jitGenerator.h"
#include "ForthDictionary.h"


inline void compileWord(const std::string& wordName, const std::string& compileText)
{
    printf("\nCompiler: compiling word: %s\n", wordName.c_str());

    JitContext& jc = JitContext::getInstance();
    jc.resetContext();

    // Start compiling the new word
    JitGenerator::genPrologue();
    const auto words = split(compileText);

    size_t i = 0;
    while (i < words.size())
    {
        const auto& word = to_lower(words[i]);
        printf("Compiler ... processing word: %s\n", word.c_str());

        auto* fword = d.findWord(word.c_str());
        if (fword) {
            if (fword->generatorFunc) {
                printf("Generating code for word: %s\n", word.c_str());
                fword->generatorFunc();
            } else if (fword->compiledFunc) {
                printf("Generating call for compiled function of word: %s\n", word.c_str());
                // Assuming JitGenerator::genCall is the method to generate call for compiled function
                JitGenerator::genCall(fword->compiledFunc);
            } else {
                printf("Error: Unknown behavior for word: %s\n", word.c_str());
                jc.resetContext();
                return;
            }
        } else if (is_number(word)) {
            uint64_t number = std::stoll(word.c_str());
            jc.uint64_A = number;
            JitGenerator::genPushLong();
            printf("Generated code for number: %s\n", word.c_str());
        } else {
            std::cout << "Error: Unknown or uncompilable word: " << word << std::endl;
            jc.resetContext();
            return;
        }
        ++i;
    }

    // Check if the word already exists in the dictionary
    if (d.findWord(wordName.c_str()) != nullptr) {
        printf("Compiler: word already exists: %s\n", wordName.c_str());
        jc.resetContext();
        return;
    }

    // Finalize compiled word
    JitGenerator::genEpilogue();
    const ForthFunction f = JitGenerator::end();
    d.addWord(wordName.c_str(), nullptr, f, nullptr);
    printf("Compiler: successfully compiled word: %s\n", wordName.c_str());
    std::cout << "Code size: " << jc.code.codeSize() << std::endl;
}

#endif //INTERPRETER_H
