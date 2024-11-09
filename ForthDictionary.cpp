#include "ForthDictionary.h"
#include <cctype>
#include <stdexcept>
#include <iostream>

#include "JitGenerator.h"

// Static method to get the singleton instance
ForthDictionary& ForthDictionary::getInstance(size_t size)
{
    static ForthDictionary instance(size);
    return instance;
}

// Private constructor to prevent instantiation
ForthDictionary::ForthDictionary(size_t size) : memory(size), currentPos(0), latestWord(nullptr)
{
}

// Add a new word to the dictionary
void ForthDictionary::addWord(const char* name,
                              ForthFunction generatorFunc,
                              ForthFunction compiledFunc,
                              ForthFunction immediateFunc,
                              ForthFunction immTerpFunc)
{
    std::string lower_name = to_lower(name);

   //  std::cout << "Added word " << lower_name << std::endl;

    if (currentPos + sizeof(ForthWord) > memory.size())
    {
        throw std::runtime_error("Dictionary memory overflow");
    }

    void* wordMemory = &memory[currentPos];
    auto* newWord = new(wordMemory) ForthWord(lower_name.c_str(),
                                              generatorFunc,
                                              compiledFunc,
                                              immediateFunc,
                                              immTerpFunc,
                                              latestWord);

    // Correctly set the latest word to the new word
    latestWord = newWord;


    currentPos += sizeof(ForthWord);

    // Can leave a comment describing why you are allotting extra space
    allot(16); // Increase current position for potentially more data
}

// Find a word in the dictionary
ForthWord* ForthDictionary::findWord(const char* name) const
{
    std::string lower_name = to_lower(name);
    ForthWord* word = latestWord;
    while (word != nullptr)
    {
        if (std::strcmp(word->name, lower_name.c_str()) == 0)
        {
            return word;
        }
        word = word->link;
    }
    return nullptr;
}

// Allot space in the dictionary
void ForthDictionary::allot(size_t bytes)
{
    if (currentPos + bytes > memory.size())
    {
        throw std::runtime_error("Dictionary memory overflow");
    }
    currentPos += bytes;
}

// Store data in the dictionary
void ForthDictionary::storeData(const void* data, size_t dataSize)
{
    if (currentPos + dataSize > memory.size())
    {
        throw std::runtime_error("Dictionary memory overflow");
    }
    std::memcpy(&memory[currentPos], data, dataSize);
    currentPos += dataSize;
}

// Get the latest added word
ForthWord* ForthDictionary::getLatestWord() const
{
    return latestWord;
}

uint64_t ForthDictionary::getCurrentPos() const
{
    return (uint64_t)currentPos;
}

uint64_t ForthDictionary::getCurrentLocation() const
{
    return reinterpret_cast<uint64_t>(&memory[currentPos]);
}

// Add base words to the dictionary
void ForthDictionary::add_base_words()
{
    // Add base words, placeholder for actual implementation
}

void ForthDictionary::forgetLastWord()
{
    if (latestWord == nullptr)
    {
        throw std::runtime_error("No words to forget");
    }

    std::cout << "Forgetting word " << latestWord->name << std::endl;

    // Size of the word (this depends on your actual implementation details. Adjust as needed).
    size_t wordSize = sizeof(ForthWord) + 16; // include the extra allotted space
    currentPos -= wordSize;

    // std::memset(&memory[currentPos], 0, wordSize); // Wipe the memory with zeros

    // Update the latest word pointer
    ForthWord* previousWord = latestWord->link;
    latestWord = previousWord;

    // Additional check to update the linking properly in dictionary
    if (latestWord != nullptr)
    {
        std::cout << "New latest word is " << latestWord->name << std::endl;
    }
    else
    {
        std::cout << "No more words in dictionary" << std::endl;
    }
}

// set the data field
void ForthDictionary::setData(uint64_t data) const
{
    latestWord->data = data;
}

void ForthDictionary::setCompiledFunction(ForthFunction func) const
{
    latestWord->compiledFunc = func;
}

void ForthDictionary::setImmediateFunction(ForthFunction func) const
{
    latestWord->immediateFunc = func;
}

void ForthDictionary::setGeneratorFunction(ForthFunction func) const
{
    latestWord->generatorFunc = func;
}

void ForthDictionary::setTerpFunction(ForthFunction func) const
{
    latestWord->terpFunc = func;
}

void ForthDictionary::setState(uint8_t i)
{
    latestWord->state = i;
}

uint8_t ForthDictionary::getState() const
{
    return latestWord->state;
}

void ForthDictionary::setName(std::string name)
{
    std::strncpy(latestWord->name, name.c_str(), sizeof(latestWord->name));
    latestWord->name[sizeof(latestWord->name) - 1] = '\0'; // Ensure null-termination
}

void ForthDictionary::setData(uint64_t d)
{
    latestWord->data = d;
}

uint64_t ForthDictionary::getData() const
{
    return latestWord->data;
}

// get and set type
uint16_t ForthDictionary::getType() const
{
    return latestWord->type;
}

void ForthDictionary::setType(const uint16_t type) const
{
   latestWord->type = type;
}


// get pointer to data
void* ForthDictionary::get_data_ptr() const
{
    return &latestWord->data;
}

void ForthDictionary::displayWord(std::string name)
{
    std::cout << "Displaying word " << name << std::endl;
    auto* word = findWord(name.c_str());
    if (word == nullptr)
    {
        std::cout << "Word not found" << std::endl;
        return;
    }
    std::cout << "Name: " << word->name << std::endl;

    // display function addresses in hex
    std::cout << "Compiled function: " << std::hex << reinterpret_cast<uintptr_t>(word->compiledFunc) << std::endl;
    std::cout << "Immediate function: " << std::hex << reinterpret_cast<uintptr_t>(word->immediateFunc) << std::endl;
    std::cout << "Generator function: " << std::hex << reinterpret_cast<uintptr_t>(word->generatorFunc) << std::endl;
    std::cout << "Interp function: " << std::hex << reinterpret_cast<uintptr_t>(word->generatorFunc) << std::endl;
    std::cout << "State: " << word->state << std::endl;
    std::cout << "Type: "  << word->type << std::endl;
    std::cout << "Data: " << std::dec << word->data << std::endl;
    std::cout << "Link: " << word->link << std::endl << std::endl;
}

// List all words in the dictionary
void ForthDictionary::list_words() const
{
    ForthWord* word = latestWord;
    while (word != nullptr)
    {
        std::cout << word->name << " ";
        word = word->link;
    }
    std::cout << std::endl;
}
