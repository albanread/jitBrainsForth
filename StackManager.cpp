#include "StackManager.h"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdexcept>

StackManager::StackManager() : stackSize(1024) {


    dsStack = static_cast<uint64_t*>(std::malloc(stackSize * sizeof(uint64_t)));
    rsStack = static_cast<uint64_t*>(std::malloc(stackSize * sizeof(uint64_t)));
    lsStack = static_cast<uint64_t*>(std::malloc(stackSize * sizeof(uint64_t)));

    if (!dsStack || !rsStack || !lsStack) {
        throw std::runtime_error("Failed to allocate memory for stacks");
    }
    dsTop = dsStack+stackSize-4;
    dsPtr = dsStack + stackSize-4;
    rsTop = rsStack + stackSize-4;
    rsPtr = rsStack + stackSize-4;
    lsTop = lsStack + stackSize-4;
    lsPtr = lsStack + stackSize-4;
}

StackManager::~StackManager() {
    std::free(dsStack);
    std::free(rsStack);
    std::free(lsStack);
}

void StackManager::display_stack() const {

    int dsDepth = dsTop-dsPtr;
    // display items from dsptr (top of stack) down 8 levels
    for (int i = 0; i< std::min(dsDepth,16); i++) {
        std::cout << "[" << i << "] = (" << *(dsPtr + i) << ")  ";
    }
    // display depth of stack
    std::cout << "Data Stack depth: " << dsDepth << std::endl;
    int rsDepth = rsTop-rsPtr;
    // display items from rsPtr (top of stack) down 8 levels
    for (int i = 0; i< std::min(rsDepth,16); i++) {
        std::cout << "rsPtr[" << i << "] = " << *(rsPtr + i);
    }
    std::cout << std::endl;
    // display depth of stack
    std::cout << "Return Stack depth: " << rsDepth << std::endl;
}

// C related functions
void StackManager::pushDS(uint64_t value) {
    if (dsPtr == dsStack) {
        throw std::runtime_error("DS stack overflow");
    }
    dsPtr--;
    *(dsPtr) = value;

}

uint64_t StackManager::popDS() {
    if (dsPtr == dsStack + stackSize) {
        throw std::runtime_error("DS stack underflow");
    }
    auto val= *(dsPtr);
    dsPtr++;
    return val;
}

void StackManager::pushRS(uint64_t value) {
    if (rsPtr == rsStack) {
        throw std::runtime_error("RS stack overflow");
    }
    *(--rsPtr) = value;
}

uint64_t StackManager::popRS() {
    if (rsPtr == rsStack + stackSize) {
        throw std::runtime_error("RS stack underflow");
    }
    return *(rsPtr++);
}