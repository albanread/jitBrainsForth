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


inline void compileWord(const std::string& wordName, const std::string& compileText, bool logging=false)
{
    // how to look at logs, and asm code.
    if (wordName == "testBeginAgain") logging=true;
    if (wordName == "testBeginAgain") jc.loggingON();

    if (logging) printf("\nCompiler v2: compiling word: [%s]\n", wordName.c_str());

    JitContext& jc = JitContext::getInstance();
    jc.resetContext();

    // Start compiling the new word
    JitGenerator::genPrologue();
    const auto words = split(compileText);

    if (logging) printf("Split words: ");
    for (const auto& word : words)
    {
        if (logging) printf("%s ", word.c_str());
    }
    if (logging) printf("\n");

    size_t i = 0;
    while (i < words.size())
    {
        const auto& word = words[i];
        if (logging) printf("Compiler ... processing word: [%s]\n", word.c_str());

        auto* fword = d.findWord(word.c_str());

        if (fword != nullptr)
        {
            if (logging) std::cout << "Found word: " << fword->name << std::endl;
            if (logging) printf("fword->compiledFunc: %p\n", fword->compiledFunc);
            if (logging) printf("fword->generatorFunc: %p\n", fword->generatorFunc);
            if (logging) printf("fword->immediateFunc: %p\n", fword->immediateFunc);
        }
        if (fword == nullptr)
        {
            if (logging) std::cout << "Word not found" << std::endl;
        }


        if (fword != nullptr) // say word exists
            if (logging) printf("Compiler: word exists: %s\n", word.c_str());

        if (fword)
        {
            if (fword->generatorFunc)
            {
                if (logging) printf("Generating code for word: %s\n", word.c_str());
                fword->generatorFunc();
            }
            else if (fword->compiledFunc)
            {
                if (logging) printf("Generating call for compiled function of word: %s\n", word.c_str());
                // Assuming JitGenerator::genCall is the method to generate call for compiled function
                JitGenerator::genCall(fword->compiledFunc);
            }
            else if (fword->immediateFunc)
            {
                if (logging) printf("Running immediate function of word: %s\n", word.c_str());
                fword->immediateFunc();
            }
            else
            {
                if (logging) printf("Error: Unknown behavior for word: %s\n", word.c_str());
                jc.resetContext();
                return;
            }
        }
        else if (is_number(word))
        {
            try
            {
                uint64_t number = std::stoll(word.c_str());
                jc.uint64_A = number;
                JitGenerator::genPushLong();
                if (logging) printf("Generated code for number: %s\n", word.c_str());
            }
            catch (const std::invalid_argument& e)
            {
                if (logging) std::cout << "Error: Invalid number: " << word << std::endl;
                jc.resetContext();
                return;
            }
            catch (const std::out_of_range& e)
            {
                if (logging) std::cout << "Error: Number out of range: " << word << std::endl;
                jc.resetContext();
                return;
            }
        }
        else
        {
            if (logging) std::cout << "Error: Unknown or uncompilable word: [" << word << "]" << std::endl;
            jc.resetContext();
            return;
        }
        ++i;
    }

    // Check if the word already exists in the dictionary
    if (d.findWord(wordName.c_str()) != nullptr)
    {
        if (logging) printf("Compiler: word already exists: %s\n", wordName.c_str());
        jc.resetContext();
        return;
    }

    // Finalize compiled word
    JitGenerator::genEpilogue();
    const ForthFunction f = JitGenerator::end();
    d.addWord(wordName.c_str(), nullptr, f, nullptr);
    if (logging) printf("Compiler: successfully compiled word: %s\n", wordName.c_str());
    if (logging) std::cout << "Code size: " << jc.code.codeSize() << std::endl;
}
#endif //INTERPRETER_H
