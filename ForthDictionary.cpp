#include "ForthDictionary.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <iostream>

// Static method to get the singleton instance
ForthDictionary& ForthDictionary::getInstance(size_t size) {
    static ForthDictionary instance(size);
    return instance;
}

// Private constructor to prevent instantiation
ForthDictionary::ForthDictionary(size_t size) : memory(size), currentPos(0), latestWord(nullptr) {}

// Add a new word to the dictionary
void ForthDictionary::addWord(const char* name, ForthFunction generatorFunc, ForthFunction compiledFunc, ForthFunction immediateFunc) {
    std::string lower_name = to_lower(name);

    std::cout << "Added word " << lower_name << std::endl;

    if (currentPos + sizeof(ForthWord) > memory.size()) {
        throw std::runtime_error("Dictionary memory overflow");
    }

    void* wordMemory = &memory[currentPos];
    auto* newWord = new (wordMemory) ForthWord(lower_name.c_str(), generatorFunc, compiledFunc, immediateFunc, latestWord);

    // Correctly set the latest word to the new word
    latestWord = newWord;

    currentPos += sizeof(ForthWord);

    // Can leave a comment describing why you are allotting extra space
    allot(16); // Increase current position for potentially more data
}

// Find a word in the dictionary
ForthWord* ForthDictionary::findWord(const char* name) const {
    std::string lower_name = to_lower(name);
    ForthWord* word = latestWord;
    while (word != nullptr) {
        if (std::strcmp(word->name, lower_name.c_str()) == 0) {
            return word;
        }
        word = word->link;
    }
    return nullptr;
}

// Allot space in the dictionary
void ForthDictionary::allot(size_t bytes) {
    if (currentPos + bytes > memory.size()) {
        throw std::runtime_error("Dictionary memory overflow");
    }
    currentPos += bytes;
}

// Store data in the dictionary
void ForthDictionary::storeData(const void* data, size_t dataSize) {
    if (currentPos + dataSize > memory.size()) {
        throw std::runtime_error("Dictionary memory overflow");
    }
    std::memcpy(&memory[currentPos], data, dataSize);
    currentPos += dataSize;
}

// Get the latest added word
ForthWord* ForthDictionary::getLatestWord() {
    return latestWord;
}

// Add base words to the dictionary
void ForthDictionary::add_base_words() {
    // Add base words, placeholder for actual implementation
}

// List all words in the dictionary
void ForthDictionary::list_words() const {
    ForthWord* word = latestWord;
    while (word != nullptr) {
        std::cout << word->name << " ";
        word = word->link;
    }
    std::cout << std::endl;
}