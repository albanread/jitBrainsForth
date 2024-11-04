#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// Forward declaration
class jitGenerator;

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
    void pushDS(uint64_t value) {
        if (dsPtr == dsStack) {
            printf("DS stack overflow\n");
            throw std::runtime_error("DS stack overflow");
        }
        dsPtr--;
        *(dsPtr) = value;
    }

    // reset DS to original state
    void resetDS() {
        dsPtr = dsTop;
        std::fill(dsStack, dsStack + 1024*1024*2, 0);
    }

    uint64_t popDS() {
        int dsDepth = dsTop - dsPtr;
        if (dsDepth<0) {
            printf("DS stack underflow\n");
            throw std::runtime_error("DS stack underflow");
        }
        auto val = *(dsPtr);
        dsPtr++;
        return val;
    }

    void pushRS(uint64_t value) {
        if (rsPtr == rsStack) {
            printf("RS stack overflow\n");
            throw std::runtime_error("RS stack overflow");
        }
        *(--rsPtr) = value;
    }

    uint64_t popRS() {
        if (rsPtr == rsStack + 1024*1024*1) {
            printf("RS stack underflow\n");
            throw std::runtime_error("RS stack underflow");
        }
        return *(rsPtr++);
    }

    [[nodiscard]] uint64_t getDStop() const {
        return reinterpret_cast<uint64_t>(dsTop);
    }

    [[nodiscard]] uint64_t getDSPtr() const {
        return reinterpret_cast<uint64_t>(dsPtr);
    }

    [[nodiscard]] uint64_t getDSDepth() const {
        const uint64_t dsDepth = dsTop - dsPtr;
        return dsDepth;
    }


    void display_stack() const {
        printf("\nData Stack: ");
        int dsDepth = dsTop - dsPtr;
        std::cout << "dsPtr = " << dsPtr << std::endl;
        std::cout << "dsTop = " << dsTop << std::endl;
        // Display items from dsPtr (top of stack) down 16 levels
        for (int i = 0; i < std::min(dsDepth, 16); i++) {
            std::cout << "[" << i << "] = (" << *(dsPtr + i) << ")  ";
        }
        // Display depth of stack
        std::cout << "Data Stack depth: " << dsDepth << std::endl;

        printf("\nReturn Stack: ");
        int rsDepth = rsTop - rsPtr;
        // Display items from rsPtr (top of stack) down 16 levels
        for (int i = 0; i < std::min(rsDepth, 16); i++) {
            std::cout << "rsPtr[" << i << "] = " << *(rsPtr + i);
        }
        // Display depth of stack
        std::cout << "Return Stack depth: " << rsDepth << std::endl;
    }

private:
    // Private constructor to prevent instantiation
    StackManager() {
        // Initialize fixed-size arrays for stacks

        std::fill(dsUnderCanary, dsUnderCanary + 1024, 9999999);
        std::fill(dsStack, dsStack + 1024*1024*2, 0);
        std::fill(dsOverCanary, dsOverCanary + 1024, 8888888);
        std::fill(rsUnderCanary, rsUnderCanary + 1024, 7777777);
        std::fill(rsStack, rsStack + 1024*1024*1, 0);
        std::fill(rsOverCanary, rsOverCanary + 1024, 66666666);
        std::fill(lsUnderCanary, lsUnderCanary + 1024, 55555555);
        std::fill(lsStack, lsStack + 1024*1024, 0);
        std::fill(lsOverCanary, lsOverCanary + 1024, 44444444);

        dsTop = dsStack + 1024*1024*2 - 4;
        dsPtr = dsStack + 1024*1024*2  - 4;
        rsTop = rsStack + 1024*1024*1  - 4;
        rsPtr = rsStack + 1024*1024*1  - 4;
        lsTop = lsStack + 1024*1024*1  - 64;
        lsPtr = lsStack + 1024*1024*1  - 4;
    }
    ~StackManager() {
        // No need to free memory since we're using fixed-size arrays
    }


    // Fixed-size arrays for stacks
    uint64_t dsUnderCanary[1024];
    uint64_t dsStack[1024*1024*2];
    uint64_t dsOverCanary[1024];
    uint64_t rsUnderCanary[1024];
    uint64_t rsStack[1024*1024*1];
    uint64_t rsOverCanary[1024];
    uint64_t lsUnderCanary[1024];
    uint64_t lsStack[1024*1024];
    uint64_t lsOverCanary[1024];

public:
    uint64_t* dsTop;
    uint64_t* dsPtr;
    uint64_t* rsTop;
    uint64_t* rsPtr;
    uint64_t* lsTop;
    uint64_t* lsPtr;

    // Friend class declaration
    friend class jitGenerator;
};



#endif // STACKMANAGER_H