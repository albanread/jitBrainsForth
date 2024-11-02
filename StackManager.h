#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include "include/asmjit/asmjit.h"
class jitGenerator; // Forward declaration


class StackManager {
public:
    // Static method to get the singleton instance
    static StackManager& getInstance() {
        static StackManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copies
    StackManager(const StackManager&) = delete;
    StackManager& operator=(const StackManager&) = delete;


    // C related functions
    void pushDS(uint64_t value);
    uint64_t popDS();
    void pushRS(uint64_t value);
    uint64_t popRS();
    uint64_t getDStop() const;

    void display_stack() const;

private:
    // Private constructor to prevent instantiation
    StackManager();
    ~StackManager();

public:
    uint64_t* dsTop;
    uint64_t* dsStack;
    uint64_t* rsTop;
    uint64_t* rsStack;
    uint64_t* lsTop;
    uint64_t* lsStack;
    uint64_t* dsPtr;
    uint64_t* rsPtr;
    uint64_t* lsPtr;
    size_t stackSize;

    friend class jitGenerator;


};

#endif // STACKMANAGER_H