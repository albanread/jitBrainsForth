#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// Forward declaration
class jitGenerator;

class StackManager
{
public:
    static StackManager& getInstance()
    {
        static StackManager instance;
        return instance;
    }

    StackManager(const StackManager&) = delete;
    StackManager& operator=(const StackManager&) = delete;

    // Data Stack Operations
    void pushDS(uint64_t value)
    {
        if (dsPtr == dsStack)
        {
            printf("DS stack overflow\n");
            throw std::runtime_error("DS stack overflow");
        }

        asm volatile (
            "mov %%r15, %0;"
            : "=r"(dsPtr) // output
        );

        dsPtr--;
        *dsPtr = value;

        asm volatile (
            "mov %0, %%r15;"
            :
            : "r"(dsPtr) // input
        );
    }

    void resetDS()
    {
        dsPtr = dsTop;
        asm volatile (
            "mov %0, %%r15;"
            :
            : "r"(dsPtr) // input
        );
        std::fill(dsStack, dsStack + 1024 * 1024 * 2, 0);
    }

    uint64_t popDS()
    {
        asm volatile (
            "mov %%r15, %0;"
            : "=r"(dsPtr) // output
        );

        int dsDepth = dsTop - dsPtr;
        if (dsDepth < 0)
        {
            printf("DS stack underflow\n");
            throw std::runtime_error("DS stack underflow");
        }
        auto val = *dsPtr;
        dsPtr++;

        asm volatile (
            "mov %0, %%r15;"
            :
            : "r"(dsPtr) // input
        );

        return val;
    }

    // Return Stack Operations
    void pushRS(uint64_t value)
    {
        if (rsPtr == rsStack)
        {
            printf("RS stack overflow\n");
            throw std::runtime_error("RS stack overflow");
        }

        asm volatile (
            "mov %%r14, %0;"
            : "=r"(rsPtr) // output
        );

        rsPtr--;
        *rsPtr = value;

        asm volatile (
            "mov %0, %%r14;"
            :
            : "r"(rsPtr) // input
        );
    }

    void resetRS()
    {
        rsPtr = rsTop;
        asm volatile (
            "mov %0, %%r14;"
            :
            : "r"(rsPtr) // input
        );
        std::fill(rsStack, rsStack + 1024 * 1024 * 1, 0);
    }

    uint64_t popRS()
    {
        asm volatile (
            "mov %%r14, %0;"
            : "=r"(rsPtr) // output
        );

        int rsDepth = rsTop - rsPtr;
        if (rsDepth < 0)
        {
            printf("RS stack underflow\n");
            throw std::runtime_error("RS stack underflow");
        }
        auto val = *rsPtr;
        rsPtr++;

        asm volatile (
            "mov %0, %%r14;"
            :
            : "r"(rsPtr) // input
        );

        return val;
    }

    // Local Stack Operations
    void pushLS(uint64_t value)
    {
        if (lsPtr == lsStack)
        {
            printf("LS stack overflow\n");
            throw std::runtime_error("LS stack overflow");
        }

        asm volatile (
            "mov %%r13, %0;"
            : "=r"(lsPtr) // output
        );

        lsPtr--;
        *lsPtr = value;

        asm volatile (
            "mov %0, %%r13;"
            :
            : "r"(lsPtr) // input
        );
    }

    void resetLS()
    {
        lsPtr = lsTop;
        asm volatile (
            "mov %0, %%r13;"
            :
            : "r"(lsPtr) // input
        );
        std::fill(lsStack, lsStack + 1024 * 1024, 0);
    }

    uint64_t popLS()
    {
        asm volatile (
            "mov %%r13, %0;"
            : "=r"(lsPtr) // output
        );

        int lsDepth = lsTop - lsPtr;
        if (lsDepth < 0)
        {
            printf("LS stack underflow\n");
            throw std::runtime_error("LS stack underflow");
        }
        auto val = *lsPtr;
        lsPtr++;

        asm volatile (
            "mov %0, %%r13;"
            :
            : "r"(lsPtr) // input
        );

        return val;
    }

    [[nodiscard]] uint64_t getDStop() const
    {
        return reinterpret_cast<uint64_t>(dsTop);
    }

    [[nodiscard]] uint64_t getDSPtr() const
    {
        return reinterpret_cast<uint64_t>(dsPtr);
    }

    [[nodiscard]] uint64_t getDSDepth() const
    {
        return dsTop - dsPtr;
    }

    [[nodiscard]] uint64_t getRStop() const
    {
        return reinterpret_cast<uint64_t>(rsTop);
    }

    [[nodiscard]] uint64_t getRSPtr() const
    {
        return reinterpret_cast<uint64_t>(rsPtr);
    }

    [[nodiscard]] uint64_t getRSDepth() const
    {
        return rsTop - rsPtr;
    }

    [[nodiscard]] uint64_t getLStop() const
    {
        return reinterpret_cast<uint64_t>(lsTop);
    }

    [[nodiscard]] uint64_t getLSPtr() const
    {
        return reinterpret_cast<uint64_t>(lsPtr);
    }

    [[nodiscard]] uint64_t getLSDepth() const
    {
        return lsTop - lsPtr;
    }

private:
    StackManager()
    {
        std::fill(dsUnderCanary, dsUnderCanary + 1024, 9999999);
        std::fill(dsStack, dsStack + 1024 * 1024 * 2, 0);
        std::fill(dsOverCanary, dsOverCanary + 1024, 8888888);
        std::fill(rsUnderCanary, rsUnderCanary + 1024, 7777777);
        std::fill(rsStack, rsStack + 1024 * 1024 * 1, 0);
        std::fill(rsOverCanary, rsOverCanary + 1024, 66666666);
        std::fill(lsUnderCanary, lsUnderCanary + 1024, 55555555);
        std::fill(lsStack, lsStack + 1024 * 1024, 0);
        std::fill(lsOverCanary, lsOverCanary + 1024, 44444444);

        dsTop = dsStack + 1024 * 1024 * 2 - 4;
        dsPtr = dsStack + 1024 * 1024 * 2 - 4;
        rsTop = rsStack + 1024 * 1024 * 1 - 4;
        rsPtr = rsStack + 1024 * 1024 * 1 - 4;
        lsTop = lsStack + 1024 * 1024 * 1 - 64;
        lsPtr = lsStack + 1024 * 1024 * 1 - 4;

        asm volatile (
            "mov %0, %%r15;"
            :
            : "r"(dsPtr) // input
        );

        asm volatile (
            "mov %0, %%r14;"
            :
            : "r"(rsPtr) // input
        );

        asm volatile (
            "mov %0, %%r13;"
            :
            : "r"(lsPtr) // input
        );
    }

    ~StackManager() = default;

    uint64_t dsUnderCanary[1024];
    uint64_t dsStack[1024 * 1024 * 2];
    uint64_t dsOverCanary[1024];
    uint64_t rsUnderCanary[1024];
    uint64_t rsStack[1024 * 1024 * 1];
    uint64_t rsOverCanary[1024];
    uint64_t lsUnderCanary[1024];
    uint64_t lsStack[1024 * 1024];
    uint64_t lsOverCanary[1024];

public:
    uint64_t* dsTop;
    uint64_t* dsPtr;
    uint64_t* rsTop;
    uint64_t* rsPtr;
    uint64_t* lsTop;
    uint64_t* lsPtr;
};

#endif // STACKMANAGER_H