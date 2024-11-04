#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
#include <iostream>
#include <cctype>

#include "utility.h"
#include "jitGenerator.h"
#include "ForthDictionary.h"
#include <unordered_set>


// Traced words set
std::unordered_set<std::string> tracedWords;

inline void traceon(const std::string& word)
{
    tracedWords.insert(word);
    std::cout << "Tracing enabled for: " << word << std::endl;
}

inline void traceoff(const std::string& word)
{
    tracedWords.erase(word);
    std::cout << "Tracing disabled for: " << word << std::endl;
}


inline void compileWord(const std::string& wordName, const std::string& compileText)
{
    bool logging = tracedWords.find(wordName) != tracedWords.end();

    if (logging)
    {
        printf("\nCompiler v2: compiling word: [%s]\n", wordName.c_str());
    }

    if (logging) printf("\nCompiler v2: compiling word: [%s]\n", wordName.c_str());

    JitContext& jc = JitContext::getInstance();
    jc.resetContext();

    if (logging)
    {
        jc.loggingON();
    }
    else
    {
        jc.loggingOFF();
    }

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
                jc.pos_next_word = i;
                jc.pos_last_word = 0;
                jc.words = &words;
                fword->immediateFunc();
                if (jc.pos_last_word != 0)
                {
                    i = jc.pos_last_word;
                }
            }
            else
            {
                if (logging) printf("Error: Unknown behavior for word: %s\n", word.c_str());
                jc.resetContext();
                return;
            }
        } else if (int o = JitGenerator::findLocal(word)!=INVALID_OFFSET)
        {
            if (logging) printf(" local variable: %s at %d\n", word.c_str(), o);
            JitGenerator::genPushLocal(jc.offset);

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
