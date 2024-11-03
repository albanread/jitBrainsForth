#include "ForthDictionary.h"

// Static method to get the singleton instance
ForthDictionary& ForthDictionary::getInstance(size_t size) {
    static ForthDictionary instance(size);
    return instance;
}

// Private constructor to prevent instantiation
ForthDictionary::ForthDictionary(size_t size) : memory(size), currentPos(0), latestWord(nullptr) {}

// Add a new word to the dictionary
void ForthDictionary::addWord(const char* name, ForthFunction generatorFunc, ForthFunction compiledFunc, ForthFunction immediateFunc) {

     printf("Added word %s\n", name);

    if (currentPos + sizeof(ForthWord) > memory.size()) {
        throw std::runtime_error("Dictionary memory overflow");
    }

    void* wordMemory = &memory[currentPos];
    ForthWord* newWord = new (wordMemory) ForthWord(name, generatorFunc, compiledFunc, immediateFunc, latestWord);

    // Correctly set the latest word to the new word
    latestWord = newWord;

    currentPos += sizeof(ForthWord);
    allot(16);
}

// Find a word in the dictionary
ForthWord* ForthDictionary::findWord(const char* name) const {
    ForthWord* word = latestWord;
    while (word != nullptr) {
        if (std::strcmp(word->name, name) == 0) {
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

// H in forth is the next free space in the dictionary as an absolute address
void* ForthDictionary::getH() const {
    return (void*)&memory[currentPos];
}

ForthWord* ForthDictionary::getLatestWord()
{
    return latestWord;
}

void ForthDictionary::add_base_words()
{




}

// list all words
void ForthDictionary::list_words()
{
    ForthWord* word = latestWord;
    while (word != nullptr) {
        printf("%s ", word->name);
        word = word->link;
    }
}
