#ifndef FORTH_DICTIONARY_H
#define FORTH_DICTIONARY_H

#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>
#include "utility.h"

// Function pointer type
typedef void (*ForthFunction)();

enum ForthWordState {
    NORMAL = 0,
    IMMEDIATE = 1 << 0,
    COMPILE_ONLY = 1 << 1,
    INTERPRET_ONLY = 1 << 2,
};

// Structure to represent a word in the dictionary
struct ForthWord {
    char name[32]{};                 // Name of the word (fixed length for simplicity)
    ForthFunction compiledFunc;      // Compiled Forth function pointer
    ForthFunction generatorFunc;     // Used to generate 'inline' code.
    ForthFunction immediateFunc;     // Immediate function pointer (if any)
    ForthWord* link;                 // Pointer to the previous word in the dictionary
    uint8_t state;                   // State of the word

    // Constructor to initialize a word
    ForthWord(const char* wordName, ForthFunction genny, ForthFunction func, ForthFunction immFunc, ForthWord* prev)
        : generatorFunc(genny), compiledFunc(func), immediateFunc(immFunc), link(prev), state(ForthWordState::NORMAL)

    {
        std::strncpy(name, wordName, sizeof(name));
        name[sizeof(name) - 1] = '\0'; // Ensure null-termination
    }
};

// Class to manage the Forth dictionary
class ForthDictionary {
public:
    // Static method to get the singleton instance
    static ForthDictionary& getInstance(size_t size = 1024*1024*8);

    // Delete copy constructor and assignment operator to prevent copies
    ForthDictionary(const ForthDictionary&) = delete;
    ForthDictionary& operator=(const ForthDictionary&) = delete;

    // Add a new word to the dictionary
    void addWord(const char* name, ForthFunction generatorFunc, ForthFunction compiledFunc, ForthFunction immediateFunc);

    // Find a word in the dictionary
    ForthWord* findWord(const char* name) const;

    // Allot space in the dictionary
    void allot(size_t bytes);

    // Store data in the dictionary
    void storeData(const void* data, size_t dataSize);

    // Get the latest added word
    ForthWord* getLatestWord();

    // Add base words to the dictionary
    void add_base_words();
    void forgetLastWord();

    // List all words in the dictionary
    void list_words() const;

private:
    // Private constructor to prevent instantiation
    explicit ForthDictionary(size_t size);

    std::vector<char> memory;    // Memory buffer for the dictionary
    size_t currentPos;           // Current position in the memory buffer
    ForthWord* latestWord;       // Pointer to the latest added word



};

#endif // FORTH_DICTIONARY_H