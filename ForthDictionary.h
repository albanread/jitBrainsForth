#ifndef FORTH_DICTIONARY_H
#define FORTH_DICTIONARY_H

#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "utility.h"

// Function pointer type
typedef void (*ForthFunction)();

enum ForthWordState
{
    NORMAL = 0,
    IMMEDIATE = 1 << 0,
    COMPILE_ONLY = 1 << 1,
    INTERPRET_ONLY = 1 << 2,
};

enum ForthWordType
{
    WORD = 0,
    CONSTANT = 1 << 0,
    VARIABLE = 1 << 1,
    VALUE = 1 << 2,
    STRING = 1 << 3
};

// Structure to represent a word in the dictionary
struct ForthWord
{
    char name[32]{}; // Name of the word (fixed length for simplicity)
    ForthFunction compiledFunc; // Compiled Forth function pointer/ executed by interpreter, called by compiler.
    ForthFunction generatorFunc; // Used to generate 'inline' code, only used by compiler.
    ForthFunction immediateFunc; // Immediate function pointer only used by compiler.
    ForthFunction terpFunc; // Immediate function pointer only used by interpreter
    ForthWord* link; // Pointer to the previous word in the dictionary
    uint8_t state; // State of the word
    uint8_t reserved; // Reserved for future use
    uint8_t type;
    uint64_t data; // New 64-bit value for storing data

    // Constructor to initialize a word
    ForthWord(const char* wordName,
              ForthFunction genny,
              ForthFunction func,
              ForthFunction immFunc,
              ForthFunction terpFunc,
              ForthWord* prev)
        : generatorFunc(genny), compiledFunc(func),
          immediateFunc(immFunc), terpFunc(terpFunc),
          link(prev), state(0), data(0) // Set data to zero
    {
        std::strncpy(name, wordName, sizeof(name));
        name[sizeof(name) - 1] = '\0'; // Ensure null-termination
    }
};

// Class to manage the Forth dictionary
class ForthDictionary
{
public:
    // Static method to get the singleton instance
    static ForthDictionary& getInstance(size_t size = 1024 * 1024 * 8);

    // Delete copy constructor and assignment operator to prevent copies
    ForthDictionary(const ForthDictionary&) = delete;
    ForthDictionary& operator=(const ForthDictionary&) = delete;

    // Add a new word to the dictionary
    void addWord(const char* name, ForthFunction generatorFunc, ForthFunction compiledFunc, ForthFunction immediateFunc,
                 ForthFunction immTerpFunc, const std::string& sourceCode);
    void addWord(const char* name, ForthFunction generatorFunc, ForthFunction compiledFunc, ForthFunction immediateFunc,
                 ForthFunction immTerpFunc);

    // Find a word in the dictionary
    ForthWord* findWord(const char* name) const;

    // Allot space in the dictionary
    void allot(size_t bytes);

    // Store data in the dictionary
    void storeData(const void* data, size_t dataSize);

    // Get the latest added word
    [[nodiscard]] ForthWord* getLatestWord() const;
    [[nodiscard]] uint64_t getCurrentPos() const;
    [[nodiscard]] uint64_t getCurrentLocation() const;

    // Add base words to the dictionary
    static void add_base_words();
    void forgetLastWord();
    void setData(uint64_t data) const;
    void setCompiledFunction(ForthFunction func) const;
    void setImmediateFunction(ForthFunction func) const;
    void setGeneratorFunction(ForthFunction func) const;
    void setTerpFunction(ForthFunction func) const;
    void setState(uint8_t i);
    [[nodiscard]] uint8_t getState() const;
    void setName(std::string name);
    void setData(uint64_t d);
    uint64_t getData() const;
    uint16_t getType() const;
    void setType(uint16_t type) const;
    void* get_data_ptr() const;
    void displayWord(std::string name);
    void SetState(uint8_t i);

    // List all words in the dictionary
    void list_words() const;

private:
    // Private constructor to prevent instantiation
    explicit ForthDictionary(size_t size);

    std::vector<char> memory; // Memory buffer for the dictionary
    size_t currentPos; // Current position in the memory buffer
    ForthWord* latestWord; // Pointer to the latest added word

    // Map to store the source code associated with each word
    std::unordered_map<std::string, std::string> sourceCodeMap;
};

#endif // FORTH_DICTIONARY_H