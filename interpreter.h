#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <regex>
#include <sstream>
#include "utility.h"
#include "ForthDictionary.h"
#include <unordered_set>
#include "StringInterner.h"
#include "JitContext.h"
#include "JitGenerator.h"

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

inline std::string scanForLiterals(const std::string& compileText) {
    std::regex literalRegex(R"((\w*")\s*(.*?)\")");
    std::string result;
    std::smatch match;
    std::string tmpText = compileText;
    StringInterner& interner = StringInterner::getInstance();

    while (std::regex_search(tmpText, match, literalRegex)) {
        std::string literalStart = match[1].str();
        std::string literalString = match[2].str();

        auto internedPtr = interner.intern(literalString);
        std::ostringstream replacementWord;
        replacementWord << "sPtr_" << reinterpret_cast<std::uintptr_t>(internedPtr);

        result += match.prefix().str();
        result += literalStart + " " + replacementWord.str() + " ";

        tmpText = match.suffix().str();
    }
    result += tmpText;

    return result;
}

inline void compileWord(const std::string& wordName, const std::string& compileText)
{
    bool logging = tracedWords.find(wordName) != tracedWords.end();

    if (logging)
    {
        printf("\nCompiling word: [%s]\n", wordName.c_str());
    }

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
                if (logging)  printf("next word is %s", words[i].c_str());
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
    const ForthFunction f = JitGenerator::endGeneration();
    d.addWord(wordName.c_str(), nullptr, f, nullptr, nullptr);
    if (logging)
    {
        printf("Compiler: successfully compiled word: %s\n", wordName.c_str());

        jc.reportMemoryUsage();
    }
}

// interpreter calls words, or pushes numbers.
inline void interpreter(const std::string& input)
{

    std::string newCompileText = scanForLiterals(input);

    const auto words = split(newCompileText);

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
                else if (fword->terpFunc) // these are immediate words.
                {
                    if (logging) printf("Running interpreter immediate word: %s\n", word.c_str());
                    jc.pos_next_word = i;
                    jc.pos_last_word = 0;
                    jc.words = &words;
                    exec(fword->terpFunc);
                    if (jc.pos_last_word != 0)
                    {
                        i = jc.pos_last_word;
                    }
                    if (logging)  printf("next word is %s", words[i].c_str());
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

    // The infinite terminal loop
    while (true)
    {
        std::cout << (compiling ? "] " : "> ");
        std::getline(std::cin, input); // Read a line of input from the terminal

        if (input == "QUIT" || input == "quit")
        {
            sm.resetDS();
            break; // Exit the loop if the user enters QUIT
        }
        if (input == "*MEM" || input == "*mem")
        {
            jc.reportMemoryUsage();
            input = "";
            continue;
        }
        if (input == "*STRINGS" || input == "*strings")
        {
            strIntern.display_list();
            input = "";
            continue;
        }

        if (input == "*QUIT" || input == "*quit")
        {
            exit(0);
        }

        if (input.empty())
        {
            continue;
        }

        accumulated_input += " " + input; // Accumulate input lines

        auto words = split(input);

        // Check if compiling is required based on the input
        for (auto it = words.begin(); it != words.end(); ++it)
        {
            const auto& word = *it;

            if (word == "QUIT" || word == "quit")
            {
                break;
            }

            if (word == "*TRON" || word == "*tron" || word == "*TROFF" || word == "*troff")
            {
                auto command = word;
                ++it; // Advance the iterator
                if (it != words.end())
                {
                    const auto& nextWord = *it;
                    if (command == "*TRON" || command == "*tron")
                    {
                        traceon(nextWord);
                    }
                    else if (command == "*TROFF" || command == "*troff")
                    {
                        traceoff(nextWord);
                    }
                    // Remove `command` and `nextWord` from accumulated_input
                    accumulated_input.erase(accumulated_input.find(command), command.length() + nextWord.length() + 2);
                }
                else
                {
                    std::cerr << "Error: Expected name of word to trace after " << command << std::endl;
                }
                continue; // Process next word
            }
            // add *optLoopCheckOn takes the following word as ON, OFF
            if (word == "*LOOPCHECK" || word == "*loopcheck")
            {
                // get next word
                ++it;
                if (it != words.end())
                {
                    const auto& nextWord = *it;
                    if (nextWord == "ON" || nextWord == "on")
                    {
                        // display loop checking on
                        std::cout << "Loop checking ON" << std::endl;
                        jc.loopCheckON();
                    }
                    else if (nextWord == "OFF" || nextWord == "off")
                    {
                        // display loop checking off
                        std::cout << "Loop checking OFF" << std::endl;
                        jc.loopCheckOFF();
                    }
                    else
                    {
                        std::cerr << "Error: Expected argument (on,off) after " << word << std::endl;
                    }
                    // Remove `command` and `nextWord` from accumulated_input
                    accumulated_input.erase(accumulated_input.find(word), word.length() + nextWord.length() + 2);
                }
            }

            if (word == "*dump" || word == "*DUMP") {
                // Get the next word
                ++it;
                if (it != words.end()) {
                    const auto& addrStr = *it;
                    uintptr_t address;

                    try {
                        if (addrStr.find("0x") != std::string::npos || addrStr.find("0X") != std::string::npos) {
                            // Address is in hexadecimal
                            address = std::stoull(addrStr, nullptr, 16);
                        } else {
                            // Address is in decimal
                            address = std::stoull(addrStr, nullptr, 10);
                        }

                        // Cast the address to a void pointer and call dump
                        dump(reinterpret_cast<void*>(address));
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Error: Invalid address format" << std::endl;
                                    } catch (const std::out_of_range& e) {
                        std::cerr << "Error: Address out of range" << std::endl;
                    }

                    // Remove `command` and `nextWord` from accumulated_input
                    accumulated_input.erase(accumulated_input.find(word), word.length() + addrStr.length() + 2);
                } else {
                    std::cerr << "Error: Expected address after " << word << std::endl;
                }
                continue; // Process next word
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

#endif //INTERPRETER_H
