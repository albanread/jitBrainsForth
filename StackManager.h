#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <array>
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

    // String Stack Operations
    void pushSS(uint64_t value)
    {
        if (ssPtr == ssStack)
        {
            printf("SS stack overflow\n");
            throw std::runtime_error("SS stack overflow");
        }

        asm volatile (
            "mov %%r12, %0;"
            : "=r"(ssPtr) // output
        );

        ssPtr--;
        *ssPtr = value;

        asm volatile (
            "mov %0, %%r12;"
            :
            : "r"(ssPtr) // input
        );
    }

    void resetSS()
    {
        ssPtr = ssTop;
        asm volatile (
            "mov %0, %%r12;"
            :
            : "r"(ssPtr) // input
        );
        std::fill(ssStack, ssStack + 1024 * 1024, 0);
    }

    uint64_t popSS()
    {
        asm volatile (
            "mov %%r12, %0;"
            : "=r"(ssPtr) // output
        );

        int ssDepth = ssTop - ssPtr;
        if (ssDepth < 0)
        {
            printf("SS stack underflow\n");
            throw std::runtime_error("SS stack underflow");
        }
        auto val = *ssPtr;
        ssPtr++;

        asm volatile (
            "mov %0, %%r12;"
            :
            : "r"(ssPtr) // input
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
        uint64_t currentDsPtr;

        // Get the current value of dsPtr from r15 register
        asm volatile (
            "mov %%r15, %0;"
            : "=r"(currentDsPtr) // output
        );

        uint64_t dsDepth = (reinterpret_cast<uint64_t>(dsTop) - reinterpret_cast<uint64_t>(currentDsPtr));
        if (dsDepth ==0) return 0;
        return (dsDepth/8);
    }

    [[nodiscard]] uint64_t getDSDepthInBytes() const
    {
        uint64_t currentDsPtr;

        // Get the current value of dsPtr from r15 register
        asm volatile (
            "mov %%r15, %0;"
            : "=r"(currentDsPtr) // output
        );

        uint64_t dsDepth = (reinterpret_cast<uint64_t>(dsTop) - reinterpret_cast<uint64_t>(currentDsPtr));
        return dsDepth;
    }


    [[nodiscard]] uint64_t getRStop() const
    {
        return reinterpret_cast<uint64_t>(rsTop);
    }

    [[nodiscard]] static uint64_t getRSPtr()
    {
        uint64_t currentRsPtr;

        // Get the current value of dsPtr from r15 register
        asm volatile (
            "mov %%r14, %0;"
            : "=r"(currentRsPtr) // output
        );

        return currentRsPtr;
    }

    [[nodiscard]] uint64_t getRSDepth() const
    {
        uint64_t currentRsPtr;

        // Get the current value of dsPtr from r14 register
        asm volatile (
            "mov %%r14, %0;"
            : "=r"(currentRsPtr) // output
        );

        uint64_t rsDepth = (reinterpret_cast<uint64_t>(rsTop) - reinterpret_cast<uint64_t>(currentRsPtr));
        if (rsDepth ==0) return 0;
        return (rsDepth/8);
    }

    [[nodiscard]] uint64_t getRSDepthInBytes() const
    {
        uint64_t currentRsPtr;

        // Get the current value of dsPtr from r14 register
        asm volatile (
            "mov %%r14, %0;"
            : "=r"(currentRsPtr) // output
        );

        uint64_t rsDepth = (reinterpret_cast<uint64_t>(rsTop) - reinterpret_cast<uint64_t>(currentRsPtr));
        return rsDepth;
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

    [[nodiscard]] uint64_t getSStop() const
    {
        return reinterpret_cast<uint64_t>(ssTop);
    }

    [[nodiscard]] uint64_t getSSPtr() const
    {
        return reinterpret_cast<uint64_t>(ssPtr);
    }

    [[nodiscard]] uint64_t getSSDepth() const
    {
        uint64_t currentSsPtr;

        // Get the current value of ssPtr from r12 register
        asm volatile (
            "mov %%r12, %0;"
            : "=r"(currentSsPtr) // output
        );

        uint64_t ssDepth = (reinterpret_cast<uint64_t>(ssTop) - reinterpret_cast<uint64_t>(currentSsPtr));
        if (ssDepth == 0) return 0;
        return (ssDepth / 8);
    }

    void displayStacks() const
    {
        uint64_t* currentDsPtr;
        uint64_t* currentRsPtr;
        uint64_t* currentSsPtr;

        // Get the current values of dsPtr, rsPtr and ssPtr from the registers
        asm volatile (
            "mov %%r15, %0;"
            : "=r"(currentDsPtr) // output
        );

        asm volatile (
            "mov %%r14, %0;"
            : "=r"(currentRsPtr) // output
        );

        asm volatile (
            "mov %%r12, %0;"
            : "=r"(currentSsPtr) // output
        );

        auto getStackValues = [](const uint64_t* top, const uint64_t* ptr, int numValues) -> std::vector<int64_t> {
            std::vector<int64_t> values(numValues, 0);
            int depth = top - ptr;
            for (int i = 0; i < numValues && i < depth; ++i) {
                values[i] = *(ptr + i);
            }
            return values;
        };

        auto dsValues = getStackValues(dsTop, currentDsPtr, 4);
        auto rsValues = getStackValues(rsTop, currentRsPtr, 4);
        auto ssValues = getStackValues(ssTop, currentSsPtr, 8);

        std::cout << "\tDS \t RS \tDS (1)\tDS (2)\tDS (3)\tDS (4)\tRS (1)\tRS (2)\tRS (3)\tRS (4)\n";
        std::cout << "\t" << getDSDepth() << "\t" << getRSDepth() << "\t" << dsValues[0] << "\t" << dsValues[1] << "\t"
                  << dsValues[2] << "\t" << dsValues[3] << "\t"
                  << rsValues[0] << "\t" << rsValues[1] << "\t"
                  << rsValues[2] << "\t" << rsValues[3] << "\n";

        std::cout << "\t" << getSSDepth() << "\t"   << "\tSS (1)\tSS (2)\tSS (3)\tSS (4)\tSS (5)\tSS (6)\tSS (7)\tSS (8)\n";
        std::cout << "\t" << "\t" << "\t" << ssValues[0] << "\t" << ssValues[1] << "\t" << ssValues[2] << "\t" << ssValues[3] << "\t"
                  << ssValues[4] << "\t" << ssValues[5] << "\t" << ssValues[6] << "\t" << ssValues[7] << "\n"
                  << std::endl;
    }

private:
    StackManager()
    {
        std::fill(dsUnderCanary, dsUnderCanary + 1024, 9999999);
        std::fill(dsStack, dsStack + 1024 * 1024 * 2, 0);
        std::fill(dsOverCanary, dsOverCanary + 1024, 8888888);
        std::fill(rsUnderCanary, rsUnderCanary + 1024, 7777777);
        std::fill(rsStack, rsStack + 1024 * 1024 * 1, 0);
        std::fill(rsOverCanary, rsOverCanary + 1024, 6666666);
        std::fill(lsUnderCanary, lsUnderCanary + 1024, 5555555);
        std::fill(lsStack, lsStack + 1024 * 1024, 0);
        std::fill(lsOverCanary, lsOverCanary + 1024, 4444444);
        std::fill(ssUnderCanary, ssUnderCanary + 1024, 3333333);
        std::fill(ssStack, ssStack + 1024 * 1024, 0);
        std::fill(ssOverCanary, ssOverCanary + 1024, 2222222);

        dsTop = dsStack + 1024 * 1024 * 2 - 4;
        dsPtr = dsStack + 1024 * 1024 * 2 - 4;
        rsTop = rsStack + 1024 * 1024 * 1 - 4;
        rsPtr = rsStack + 1024 * 1024 * 1 - 4;
        lsTop = lsStack + 1024 * 1024 * 1 - 4;
        lsPtr = lsStack + 1024 * 1024 * 1 - 4;
        ssTop = ssStack + 1024 * 1024 - 4;
        ssPtr = ssStack + 1024 * 1024 - 4;

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

        asm volatile (
            "mov %0, %%r12;"
            :
            : "r"(ssPtr) // input
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
    uint64_t ssUnderCanary[1024];
    uint64_t ssStack[1024 * 1024];
    uint64_t ssOverCanary[1024];

public:
    uint64_t* dsTop;
    uint64_t* dsPtr;
    uint64_t* rsTop;
    uint64_t* rsPtr;
    uint64_t* lsTop;
    uint64_t* lsPtr;
    uint64_t* ssTop;
    uint64_t* ssPtr;
};

#endif // STACKMANAGER_H