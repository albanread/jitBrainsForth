#ifndef JITCONTEXT_H
#define JITCONTEXT_H

#include <iostream>
#include "include/asmjit/asmjit.h"
#include <string>
#include <vector>

class JitContext
{
public:
    // Static method to get the singleton instance
    static JitContext& getInstance()
    {
        static JitContext instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copies
    JitContext(const JitContext&) = delete;
    JitContext& operator=(const JitContext&) = delete;

    // Method to get the assembler reference
    [[nodiscard]] asmjit::x86::Assembler& getAssembler() const
    {
        return *assembler;
    }

    void resetContext()
    {
        if (auto_reset)
        {
            // Reset and reinitialize the code holder
            code.reset();
            code.init(rt.environment());
            //
            // // Recreate the assembler with the new code holder
            // //delete assembler;
            assembler = new asmjit::x86::Assembler(&code);
            if (logging)
            {
                code.setLogger(&logger);
            }

            if (logging) std::cout << "AsmJit context has been reset and reinitialized." << std::endl;
        }
    }

    // Example method
    static void someJitFunction()
    {
        // Implementation of a method
        std::cout << "Executing some JIT function..." << std::endl;
    }

    void loggingON()
    {
        logging = true;
        code.setLogger(&logger);
    }

    void loggingOFF()
    {
        logging = false;
        code.setLogger(nullptr); // Disable logging

    }

    void resetON()
    {
        auto_reset = true;
    }

    void resetOFF()
    {
        auto_reset = false;
    }

private:
    // Private constructor to prevent instantiation
JitContext() :
    rt(),
    assembler(nullptr),

    logger(stdout),
    uint64_A(0),
    uint64_B(0),
    uint32_A(0),
    uint32_B(0),
    uint16(0),
    uint8(0),
    int64_A(0),
    int64_B(0),
    int32(0),
    int16(0),
    int8(0),
    offset(0),
    f(0.0f),
    d(0.0),
    ptr_A(nullptr),
    ptr_B(nullptr),
    logging(false),
    pos_next_word(0),
    pos_last_word(0),
    words(nullptr),
    word()

{
        // Initialization code
        code.reset();
        code.init(rt.environment());
        assembler = new asmjit::x86::Assembler(&code);
        if (logging)
        {
            code.setLogger(&logger);
        }
    }

    // Private destructor
    ~JitContext()
    {
        // Cleanup code
        delete assembler;
    }

public:
    asmjit::FileLogger logger; // Logs to the standard output
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::x86::Assembler* assembler;
    asmjit::Label epilogueLabel;

    // Used to pass arguments to the code generators
    uint64_t uint64_A;
    uint64_t uint64_B;
    uint32_t uint32_A;
    uint32_t uint32_B;
    uint16_t uint16;
    uint8_t uint8;
    int64_t int64_A;
    int64_t int64_B;
    int32_t int32;
    int16_t int16;
    int8_t int8;
    int offset;
    float f;
    double d;
    void* ptr_A;
    void* ptr_B;
    bool logging = false;
    bool auto_reset = true;
    // these are for immediate words that read the input stream
    size_t pos_next_word;
    size_t pos_last_word;
    const std::vector<std::string>* words;
    std::string word;


};

#endif // JITCONTEXT_H
