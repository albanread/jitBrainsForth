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
inline std::unordered_set<std::string> tracedWords;

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

inline void clearR15()
{
    asm volatile (
        "xor %%r15, %%r15"
        :
        :
        : "r15"
    );
}
inline void exec(ForthFunction f)
{
    clearR15();
    f();
}


inline void compileWord(const std::string& wordName, const std::string& compileText)
{

    //traceon("test");
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
               exec(fword->generatorFunc);
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
                exec(fword->immediateFunc);
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
        }
        else if (int o = JitGenerator::findLocal(word) != INVALID_OFFSET)
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
            throw std::runtime_error("Compiler Error: Unknown or uncompilable word: " + word);
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

// interpreter calls words, or pushes numbers.
inline void interpreter(const std::string& input)
{
    //bool logging = true;
    const auto words = split(input);
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

        if (logging) printf("Interpreter ... processing word: [%s]\n", word.c_str());

        if (word == ":")
        {
            if (logging) printf("Entering compile mode to define a new word.\n");
            ++i;
            std::string wordName;

            // Get the new word name
            if (i < words.size())
            {
                wordName = words[i];
                if (logging) printf("New word definition: [%s]\n", wordName.c_str());
            }
            else
            {
                if (logging) printf("Error: No word name provided after ':'\n");
                throw std::runtime_error("Interpreter Error: No word name provided after ':'");
            }

            std::string compileText;

            // Collect the words until the next ";"
            ++i;
            while (i < words.size() && words[i] != ";")
            {
                compileText += words[i] + " ";
                ++i;
            }

            if (i >= words.size())
            {
                if (logging) printf("Error: No ending ';' found for word definition.\n");
                throw std::runtime_error("Interpreter Error: No ending ';' found for word definition.");
            }

            // Compile the collected words
            compileWord(wordName, compileText);

            // Skip the ";" in the input words
            ++i;
        }
        else
        {
            auto* fword = d.findWord(word.c_str());

            // Ensure fword is a valid pointer before dereferencing
            if (fword)
            {
                if (fword->compiledFunc)
                {
                    if (logging) printf("Calling word: %s\n", word.c_str());
                    exec(fword->compiledFunc);
                }
                else if (fword->immediateFunc && fword->state > 32)
                {
                    if (logging) printf("Running immediate function of word: %s\n", word.c_str());
                    jc.pos_next_word = i;
                    jc.pos_last_word = 0;
                    jc.words = &words;
                    exec(fword->immediateFunc);
                    if (jc.pos_last_word != 0)
                    {
                        i = jc.pos_last_word;
                    }
                }
                else
                {
                    if (logging) std::cout << "Error: Word [" << word << "] found but cannot be executed.\n";
                    // display word details here

                    d.displayWord(word);

                    throw std::runtime_error("Cannot execute word: " + word);
                }
            }
            else if (is_number(word))
            {
                try
                {
                    const uint64_t number = std::stoll(word.c_str());
                    sm.pushDS(number);
                    if (logging) printf("Pushing %s\n", word.c_str());
                }
                catch (const std::invalid_argument& e)
                {
                    if (logging) std::cout << "Error: Invalid number: " << word << std::endl;
                    throw;
                }
                catch (const std::out_of_range& e)
                {
                    if (logging) std::cout << "Error: Number out of range: " << word << std::endl;
                    throw;
                }
            }
            else
            {
                if (logging) std::cout << "Error: Unknown or uncompilable word: [" << word << "]" << std::endl;
                throw std::runtime_error("Unknown word: " + word);
            }
        }
        ++i;
    }
}
inline void interactive_terminal()
{
    std::string input;
    std::string accumulated_input;
    bool compiling = false;


    // the infinite terminal loop
    while (true)
    {
        std::cout << (compiling ? "] " : "> ");
        std::getline(std::cin, input); // Read a line of input from the terminal

        if (input == "QUIT" || input == "quit")
        {
            break; // Exit the loop if the user enters QUIT
        }

        accumulated_input += " " + input; // Accumulate input lines
        auto words = split(input);
        // Check if compiling is required based on the input
        for (const auto& word : words)
        {
            if (word == "QUIT" || word == "quit")
            {
                break;
            }


            if (word == ":")
            {
                compiling = true;
            }
            else if (word == ";")
            {
                compiling = false;
                interpreter(accumulated_input);
                accumulated_input.clear();
                break;
            }
        }

        if (!compiling)
        {
            interpreter(accumulated_input); // Process the accumulated input using the outer_interpreter
            accumulated_input.clear();
            std::cout << " Ok" << std::endl;
        }
    }
}

// named quit in honour of FORTH
inline void Quit()
{
    while (true)
    {
        try
        {
            interactive_terminal();
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Runtime error: " << e.what() << std::endl;

            sm.resetDS();
            jc.resetContext();
        }
        // reset on quit

        sm.resetDS();
        printf("\nreset by quit\n");

    }
}

#endif //INTERPRETER_H
