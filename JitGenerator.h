#ifndef JITGENERATOR_H
#define JITGENERATOR_H

#include <functional>
#include <iostream>
#include <stdexcept>
#include "include/asmjit/asmjit.h"
#include "JitContext.h"
#include "ForthDictionary.h"
#include <stack>
#include "StackManager.h"
#include <variant>

#include "interpreter.h"
#include "quit.h"
#include "StringInterner.h"

StringInterner* strIntern = nullptr;

const int INVALID_OFFSET = -9999;

enum LoopType
{
    IF_THEN_ELSE,
    FUNCTION_ENTRY_EXIT,
    DO_LOOP,
    BEGIN_AGAIN_REPEAT_UNTIL
};

struct IfThenElseLabel
{
    asmjit::Label ifLabel;
    asmjit::Label elseLabel;
    asmjit::Label thenLabel;
    asmjit::Label exitLabel;
    asmjit::Label leaveLabel;
    bool hasElse;
    bool hasExit;
    bool hasLeave;
};

struct FunctionEntryExitLabel
{
    asmjit::Label entryLabel;
    asmjit::Label exitLabel;
};

struct DoLoopLabel
{
    asmjit::Label doLabel;
    asmjit::Label loopLabel;
    asmjit::Label leaveLabel;
    bool hasLeave;
};

struct BeginAgainRepeatUntilLabel
{
    asmjit::Label beginLabel;
    asmjit::Label againLabel;
    asmjit::Label repeatLabel;
    asmjit::Label untilLabel;
    asmjit::Label whileLabel;
    asmjit::Label leaveLabel;

    void print() const
    {
        std::cout << "Begin Label: " << beginLabel.id() << "\n";
        std::cout << "Again Label: " << againLabel.id() << "\n";
        std::cout << "Repeat Label: " << repeatLabel.id() << "\n";
        std::cout << "Until Label: " << untilLabel.id() << "\n";
        std::cout << "While Label: " << whileLabel.id() << "\n";
        std::cout << "Leave Label: " << leaveLabel.id() << "\n";
    }
};

using LabelVariant = std::variant<IfThenElseLabel, FunctionEntryExitLabel, DoLoopLabel, BeginAgainRepeatUntilLabel>;

struct LoopLabel
{
    LoopType type;
    LabelVariant label;
};

inline std::stack<LoopLabel> loopStack;

inline int doLoopDepth = 0;

inline std::stack<LoopLabel> tempLoopStack;

// save stack to tempLoopStack

static void saveStackToTemp()
{
    // Ensure tempLoopStack is empty before use
    while (!tempLoopStack.empty())
    {
        tempLoopStack.pop();
    }

    while (!loopStack.empty())
    {
        tempLoopStack.push(loopStack.top());
        loopStack.pop();
    }
}

static void restoreStackFromTemp()
{
    while (!tempLoopStack.empty())
    {
        loopStack.push(tempLoopStack.top());
        tempLoopStack.pop();
    }
}

// support locals
static int arguments_to_local_count;
static int locals_count;
static int returned_arguments_count;

struct VariableInfo
{
    std::string name;
    int offset;
};

static std::unordered_map<std::string, VariableInfo> arguments;
static std::unordered_map<std::string, VariableInfo> locals;
static std::unordered_map<std::string, VariableInfo> returnValues;
static std::unordered_map<int, std::string> argumentsByOffset;
static std::unordered_map<int, std::string> localsByOffset;
static std::unordered_map<int, std::string> returnValuesByOffset;


inline JitContext& jc = JitContext::getInstance();
inline ForthDictionary& d = ForthDictionary::getInstance();
inline StackManager& sm = StackManager::getInstance();

inline extern void prim_emit(const uint64_t a)
{
    char c = static_cast<char>(a & 0xFF);
    std::cout << c;
}

static bool logging = false;

// using ExecFunc = void (*)(ForthFunction);
// ExecFunc exec;

class JitGenerator
{
public:
    void accessStackManager(StackManager& sm);


    // Static method to get the singleton instance
    static JitGenerator& getInstance()
    {
        static JitGenerator instance;
        return instance;
    }


    // Delete copy constructor and assignment operator to prevent copies
    JitGenerator(const JitGenerator&) = delete;
    JitGenerator& operator=(const JitGenerator&) = delete;


    static void commentWithWord(const std::string& baseComment)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        const std::string comment = baseComment + " [" + jc.word + "]";
        a.comment(comment.c_str());
    }


    static void entryFunction()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- entryFunction");
        a.nop();

        // Check R15 for the value 0xA1B2C3D4
        /*a.comment(" ; Check if R15 is 0xA1B2C3D4");
        a.mov(asmjit::x86::rax, 0xA1B2C3D4);
        a.cmp(asmjit::x86::r15, asmjit::x86::rax);

        // Jump over the loading code if R15 matches the value 0xA1B2C3D4
        asmjit::Label skipLoad = a.newLabel();
        a.je(skipLoad);*/

        // Load datastack to r15
        // a.comment("; load datastack to r15");
        // a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.dsPtr));
        //a.mov(asmjit::x86::r15, asmjit::x86::qword_ptr(asmjit::x86::rax));

        // Load return stack to r14
        // a.comment("; load return stack to r14");
        // a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.rsPtr));
        // a.mov(asmjit::x86::r14, asmjit::x86::qword_ptr(asmjit::x86::rax));

        // Load local stack to r13
        // a.comment("; load local stack to r13");
        // a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.lsPtr));
        // a.mov(asmjit::x86::r13, asmjit::x86::qword_ptr(asmjit::x86::rax));

        // Label to skip to if R15 is 0xA1B2C3D4
        //a.bind(skipLoad);
    }

    static void exitFunction()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;


        /*
        // Check R15 for the value 0xA1B2C3D4
        a.comment(" ; Check if R15 is 0xA1B2C3D4");
        a.mov(asmjit::x86::rax, 0xA1B2C3D4);
        a.cmp(asmjit::x86::r15, asmjit::x86::rax);
        // Jump over the loading code if R15 matches the value 0xA1B2C3D4
        asmjit::Label skipLoad = a.newLabel();
        a.je(skipLoad);*/

        //  a.comment(" ; ----- exitFunction");
        //  a.nop();
        //  a.comment("; save datastack from r15");
        // // a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.dsPtr));
        // // a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r15);
        //  a.comment("; save return stack from r14");
        //  a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.rsPtr));
        //  a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r14);
        //  a.comment("; save local stack from r13");
        //  a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.lsPtr));
        //  a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r13);
        // Label to skip to if R15 is 0xA1B2C3D4
        //a.bind(skipLoad);
    }


    // preserve stack pointers
    static void preserveStackPointers()
    {
        // if (!jc.assembler)
        // {
        //     throw std::runtime_error("entryFunction: Assembler not initialized");
        // }
        // auto& a = *jc.assembler;
        // a.comment(" ; preserve r13, r14, r15, stack pointers on dsp");
        // // preserve r13, r14, r15 on dsp
        //
        // a.push(asmjit::x86::r8);
        // a.push(asmjit::x86::r13);
        // a.push(asmjit::x86::r14);
        // a.push(asmjit::x86::r15);
        // a.push(asmjit::x86::r15);
    }


    static void restoreStackPointers()
    {
        // if (!jc.assembler)
        // {
        //     throw std::runtime_error("entryFunction: Assembler not initialized");
        // }
        // auto& a = *jc.assembler;
        // a.comment(" ; restore r13, r14, r15, stack pointers from dsp");
        // // restore r13, r14, r15 on dsp
        // a.pop(asmjit::x86::r15);
        // a.pop(asmjit::x86::r15);
        // a.pop(asmjit::x86::r14);
        // a.pop(asmjit::x86::r13);
        // a.pop(asmjit::x86::r8);
    }


    // AsmJit related functions

    static void pushDS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- pushDS");
        a.comment(" ; save value to the data stack (r15)");
        a.sub(asmjit::x86::r15, 8);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r15), reg);
    }

    static void popDS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- popDS");
        a.comment(" ; fetch value from the data stack (r15)");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r15));
        a.add(asmjit::x86::r15, 8);
    }

    // load the value from the address
    static void loadDS(void* dataAddress)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Load the address into rax
        a.mov(asmjit::x86::rax, dataAddress);

        // Dereference the address to get the value and store it into rax
        a.mov(asmjit::x86::rax, asmjit::x86::ptr(asmjit::x86::rax));

        // Push the value onto the data stack
        pushDS(asmjit::x86::rax);
    }

    // load address from DS, fetch value and push
    static void loadFromDS()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Load the address into rax
        popDS(asmjit::x86::rax);
        // Dereference the address to get the value and store it into rax
        a.mov(asmjit::x86::rax, asmjit::x86::ptr(asmjit::x86::rax));
        // Push the value onto the data stack
        pushDS(asmjit::x86::rax);
    }


    // store the value from DS into the address specified, consumes rax.
    static void storeDS(void* dataAddress)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Pop the value from the data stack into rax
        popDS(asmjit::x86::rax);

        // Load the address into rcx
        a.mov(asmjit::x86::rcx, dataAddress);
        // Store the value from rax into the address pointed to by rcx
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rcx), asmjit::x86::rax);
    }


    // store the value from DS into the address from DS, consumes rax, rxc
    static void storeFromDS()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Pop the value from the data stack into rax
        popDS(asmjit::x86::rcx); // address
        popDS(asmjit::x86::rax); // data
        // Store the value from rax into the address pointed to by rcx
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rcx), asmjit::x86::rax);
    }


    static void pushRS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- pushRS");
        a.comment(" ; save value to the return stack (r14)");
        a.nop();
        a.sub(asmjit::x86::r14, 8);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r14), reg);
    }

    static void popRS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- popRS");
        a.comment(" ; fetch value from the return stack (r14)");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r14));
        a.add(asmjit::x86::r14, 8);
    }

    static void pushSS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- pushSS");
        a.comment(" ; save value to the string stack (r12)");
        a.sub(asmjit::x86::r12, 8);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r12), reg);
    }

    static void popSS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- popSS");
        a.comment(" ; fetch value from the string stack (r12)");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r12));
        a.add(asmjit::x86::r12, 8);
    }


    static void loadSS(void* dataAddress)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Load the address into rax
        a.mov(asmjit::x86::rax, dataAddress);

        // Dereference the address to get the value and store it into rax
        a.mov(asmjit::x86::rax, asmjit::x86::ptr(asmjit::x86::rax));

        // Push the value onto the string stack
        pushSS(asmjit::x86::rax);
    }


    static void loadFromSS()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Load the address into rax
        popSS(asmjit::x86::rax);
        // Dereference the address to get the value and store it into rax
        a.mov(asmjit::x86::rax, asmjit::x86::ptr(asmjit::x86::rax));
        // Push the value onto the string stack
        pushSS(asmjit::x86::rax);
    }

    static void storeSS(void* dataAddress)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Pop the value from the string stack into rax
        popSS(asmjit::x86::rax);

        // Load the address into rcx
        a.mov(asmjit::x86::rcx, dataAddress);
        // Store the value from rax into the address pointed to by rcx
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rcx), asmjit::x86::rax);
    }


    static void storeFromSS()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        // Pop the value from the string stack into rax
        popSS(asmjit::x86::rcx); // address
        popSS(asmjit::x86::rax); // data
        // Store the value from rax into the address pointed to by rcx
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rcx), asmjit::x86::rax);
    }


    // Function to find local by name
    static int findLocal(const std::string& word)
    {
        if (arguments.find(word) != arguments.end())
        {
            jc.offset = arguments[word].offset;
            return arguments[word].offset;
        }
        else if (locals.find(word) != locals.end())
        {
            jc.offset = locals[word].offset;
            return locals[word].offset;
        }
        else if (returnValues.find(word) != returnValues.end())
        {
            jc.offset = returnValues[word].offset;
            return returnValues[word].offset;
        }
        else
        {
            return INVALID_OFFSET;
        }
    }


    // Function to find local by offset
    static std::string findLocalByOffset(int offset)
    {
        if (argumentsByOffset.find(offset) != argumentsByOffset.end())
        {
            return argumentsByOffset[offset];
        }
        else if (localsByOffset.find(offset) != localsByOffset.end())
        {
            return localsByOffset[offset];
        }
        else if (returnValuesByOffset.find(offset) != returnValuesByOffset.end())
        {
            return returnValuesByOffset[offset];
        }
        else
        {
            return ""; // Return an empty string if not found
        }
    }

    static void addArgument(const std::string& name, int offset)
    {
        arguments[name] = {name, offset};
        argumentsByOffset[offset] = name;
    }

    static void addLocal(const std::string& name, int offset)
    {
        locals[name] = {name, offset};
        localsByOffset[offset] = name;
    }

    static void addReturnValue(const std::string& name, int offset)
    {
        returnValues[name] = {name, offset};
        returnValuesByOffset[offset] = name;
    }


    static void fetchLocal(asmjit::x86::Gp reg, int offset)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- fetchLocal");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r13, offset));
        pushDS(reg);
    }


    static void genPushLocal(int offset)
    {
        printf("genPushLocal %d\n", offset);
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;


        jc.word = findLocalByOffset(offset);

        commentWithWord(" ; ----- fetchLocal");
        asmjit::x86::Gp reg = asmjit::x86::ecx;
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r13, offset));
        pushDS(reg);
    }

    static void storeLocal(asmjit::x86::Gp reg, int offset)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- storeLocal");
        a.nop();
        popDS(reg);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r13, offset), reg);
    }

    static void allocateLocals(int count)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.sub(asmjit::x86::r13, count * 8);
    }


    static void commentWithWord(const std::string& baseComment, const std::string& word)
    {
        JitContext& jc = JitContext::getInstance();
        if (jc.assembler)
        {
            std::string comment = baseComment + " " + word;
            jc.assembler->comment(comment.c_str());
        }
    }


    // gen_leftBrace, processes the locals brace { a b | cd -- e } etc.
    static void gen_leftBrace()
    {
        JitContext& jc = JitContext::getInstance();

        if (!jc.assembler)
        {
            throw std::runtime_error("gen_leftBrace: Assembler not initialized");
        }

        // Clear previous data
        arguments.clear();
        argumentsByOffset.clear();
        locals.clear();
        localsByOffset.clear();
        returnValues.clear();
        returnValuesByOffset.clear();
        arguments_to_local_count = 0;
        locals_count = 0;
        returned_arguments_count = 0;


        auto& a = *jc.assembler;
        a.comment(" ; ----- leftBrace: locals detected");
        a.nop();

        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1; // Start just after the left brace

        enum ParsingMode
        {
            ARGUMENTS,
            LOCALS,
            RETURN_VALUES
        } mode = ARGUMENTS;

        int offset = 0;
        while (pos < words.size())
        {
            const std::string& word = words[pos];
            jc.word = word;

            if (word == "}")
            {
                break;
            }
            else if (word == "|")
            {
                mode = LOCALS;
            }
            else if (word == "--")
            {
                mode = RETURN_VALUES;
            }
            else
            {
                commentWithWord(" ; ----- prepare  ", word);
                switch (mode)
                {
                case ARGUMENTS:
                    commentWithWord(" ; ----- argument ", word);
                    addArgument(word, offset);
                    arguments_to_local_count++;
                    break;
                case LOCALS:
                    commentWithWord(" ; ----- local ", word);
                    addLocal(word, offset);
                    locals_count++;
                    break;
                case RETURN_VALUES:
                    commentWithWord(" ; ----- return value ", word);
                    addReturnValue(word, offset);
                    returned_arguments_count++;
                    break;
                }
                offset += 8;
            }
            ++pos;
        }

        /*
        printf("arguments_to_local_count: %d\n", arguments_to_local_count);
        printf("locals_count: %d\n", locals_count);
        printf("returned_arguments_count: %d\n", returned_arguments_count);
        */


        jc.pos_last_word = pos;

        // Generate locals code
        const int totalLocalsCount = arguments_to_local_count + locals_count + returned_arguments_count;
        if (totalLocalsCount > 0)
        {
            a.comment(" ; ----- allocate locals");
            allocateLocals(totalLocalsCount);

            a.comment(" ; --- BEGIN copy args to locals");
            for (int i = 0; i < arguments_to_local_count; ++i)
            {
                asmjit::x86::Gp argReg = asmjit::x86::rcx;
                int offset = i * 8; // Offsets are allocated upwards from r13.
                jc.word = findLocalByOffset(offset);
                copyLocalFromDS(argReg, offset); // Copy the argument to the return stack
            }
            a.comment(" ; --- END copy args to locals");

            a.comment(" ; --- BEGIN zero remaining locals");
            int zeroOutCount = locals_count + returned_arguments_count;
            for (int j = 0; j < zeroOutCount; ++j)
            {
                int offset = (j + arguments_to_local_count) * 8; // Offset relative to the arguments.
                jc.word = findLocalByOffset(offset);
                zeroStackLocation(offset); // Use a helper function to zero out the stack location.
            }
            a.comment(" ; --- END zero remaining locals");
        }
    }


    // genFetch - fetch the contents of the address
    static void genFetch(uint64_t address)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        asmjit::x86::Gp addr = asmjit::x86::rax; // General purpose register for address
        asmjit::x86::Gp value = asmjit::x86::rdi; // General purpose register for the value
        a.mov(addr, address); // Move the address into the register.
        a.mov(value, asmjit::x86::ptr(addr));
        pushDS(value); // Push the value onto the stack.
    }

    // display details on word
    static void see()
    {
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;
        std::string w = words[pos];
        jc.word = w;
        // display word w
        d.displayWord(w);
        jc.pos_last_word = pos;
    }


    // The TO word safely updates the container word
    // in compile mode only.
    static void genTO()
    {
        logging = true;
        jc.loggingON();

        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;

        std::string w = words[pos];
        jc.word = w;
        // This needs to be a word we can store things in.

        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        // Current assembler instance
        auto& a = *jc.assembler;

        // Check for a local variable
        int offset = findLocal(w);
        if (offset != INVALID_OFFSET)
        {
            commentWithWord("; TO ----- pop stack into local variable: ", w);

            // Pop the value from the data stack into ecx
            popDS(asmjit::x86::ecx);

            // Store the value into the local variable
            a.mov(asmjit::x86::qword_ptr(asmjit::x86::r13, offset), asmjit::x86::ecx);

            jc.pos_last_word = pos;
            return;
        }

        // Get the word from the dictionary

        auto fword = d.findWord(w.c_str());
        if (fword)
        {
            auto word_type = fword->type;
            if (word_type == ForthWordType::VALUE) // value
            {
                auto data_address = d.get_data_ptr();
                commentWithWord("; TO ----- update value: ", w);

                // Load the address of the word's data
                a.mov(asmjit::x86::rax, data_address);

                // Pop the value from the data stack into rcx
                popDS(asmjit::x86::rcx);

                // Store the value into the address
                a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::rcx);
            }
            else if (word_type == ForthWordType::VARIABLE) // variable
            {
                commentWithWord("; TO ----- pop stack into VARIABLE: ", w);

                // Get the address of the variable's data
                auto* variable_address = reinterpret_cast<int64_t*>(d.get_data_ptr());
                if (!variable_address)
                {
                    throw std::runtime_error("Failed to get variable address for word: " + w);
                }
                storeDS(variable_address);
            }
            jc.pos_last_word = pos;
        }
        else
        {
            throw std::runtime_error("Unknown word in TO: " + w);
        }

        logging = false;
        jc.loggingOFF();
    }


    // The TO word safely updates the container word
    // in interpret mode only.
    static void execTO()
    {
        logging = true;
        jc.loggingON();

        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;

        std::string w = words[pos];
        jc.word = w;


        // Get the word from the dictionary
        auto fword = d.findWord(w.c_str());
        if (fword)
        {
            auto word_type = fword->type;
            if (word_type == ForthWordType::VALUE) // value
            {
                auto data_address = d.get_data_ptr();

                // Pop the value from the data stack
                auto value = sm.popDS();

                // Store the value into the address
                *reinterpret_cast<int64_t*>(data_address) = value;
            }
            else if (word_type == ForthWordType::VARIABLE) // variable
            {
                // Load the address of the data (double indirect access)
                auto variable_address = reinterpret_cast<uint64_t>(&fword->data);


                // Pop the value from the data stack
                auto value = sm.popDS();

                // Store the value into the address the variable points to
                *reinterpret_cast<int64_t*>(variable_address) = value;
            }

            jc.pos_last_word = pos;
        }
        else
        {
            throw std::runtime_error("Unknown word in TO: " + w);
        }

        logging = false;
        jc.loggingOFF();
    }


    // immediate value, runs when value is called.
    // 10 VALUE fred
    static void genImmediateValue()
    {
        //logging = true;
        //jc.loggingON();
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;

        std::string word = words[pos];
        jc.word = word;


        // Pop the initial value from the data stack
        auto initialValue = sm.popDS();
        //printf("initialValue: %llu\n", initialValue);
        jc.resetContext();
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        commentWithWord(" ; ----- immediate value: ", word);
        // Add the word to the dictionary as a value
        d.addWord(word.c_str(), nullptr, nullptr, nullptr, nullptr);
        d.setData(initialValue); // Set the value
        auto dataAddress = d.get_data_ptr();
        d.setType(ForthWordType::VALUE); // value type

        a.comment(" ; ----- fetch value");
        loadDS(dataAddress);
        a.ret();

        ForthFunction compiledFunc = endGeneration();
        d.setCompiledFunction(compiledFunc);
        // Update position
        jc.pos_last_word = pos;
        //logging = false;
        //jc.loggingOFF();
    }


    // immediate value, runs when value is called.
    // s" literal string" VALUE fred
    static void genImmediateStringValue()
    {
        //logging = true;
        //jc.loggingON();
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;

        std::string word = words[pos];
        jc.word = word;


        // Pop the initial value from the data stack
        auto initialValue = sm.popDS();
        printf("initialValue: %llu\n", initialValue);
        jc.resetContext();
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        commentWithWord(" ; ----- immediate value: ", word);
        // Add the word to the dictionary as a value
        d.addWord(word.c_str(), nullptr, nullptr, nullptr, nullptr);
        d.setData(initialValue); // Set the value
        auto dataAddress = d.get_data_ptr();
        d.setType(ForthWordType::STRING); // value type

        a.comment(" ; ----- fetch value");
        loadSS(dataAddress);
        a.ret();

        ForthFunction compiledFunc = endGeneration();
        d.setCompiledFunction(compiledFunc);
        // Update position
        jc.pos_last_word = pos;
        //logging = false;
        //jc.loggingOFF();
    }


    static void genImmediateVariable()
    {
        //logging = true;
        //jc.loggingON();
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;

        std::string word = words[pos];
        jc.word = word;

        jc.resetContext();
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        commentWithWord(" ; ----- immediate value: ", word);
        // Add the word to the dictionary as a value
        d.addWord(word.c_str(), nullptr, nullptr, nullptr, nullptr);
        d.setData(0); // Set the value
        d.setType(ForthWordType::VARIABLE); // variable type

        auto dataAddress = d.get_data_ptr();
        // Generate prologue for function
        //printf("dataAddress: %llu\n", dataAddress);
        // use the data address to fetch the value
        a.comment(" ; ----- fetch variable address ");
        a.mov(asmjit::x86::rax, dataAddress);
        pushDS(asmjit::x86::rax);
        a.ret();
        ForthFunction compiledFunc = endGeneration();
        d.setCompiledFunction(compiledFunc);
        // Update position
        jc.pos_last_word = pos;
        //logging = false;
        //jc.loggingOFF();
    }


    static size_t stripIndex(const std::string& token)
    {
        std::string prefix = "sPtr_";
        if (token.compare(0, prefix.size(), prefix) == 0)
        {
            // Extract the numeric part after the prefix
            std::string numberStr = token.substr(prefix.size());
            std::uintptr_t address = 0;

            // Convert the numeric string to an unsigned integer
            std::istringstream iss(numberStr);
            iss >> address;

            // Return the address as a pointer to const char
            return address;
        }

        // Return nullptr if the prefix does not match
        return -1;
    }

    // supports ."
    static void genImmediateDotQuote()
    {
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;
        std::string word = words[pos];
        jc.word = word;
        if (logging) printf("genImmediateDotQuote: %s\n", word.c_str());
        const auto index = stripIndex(word);
        auto address = strIntern.getStringAddress(index);

        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        commentWithWord(" ; ----- .\" displaying text ");

        // put parameter in argument
        a.mov(asmjit::x86::rcx, address);
        // Allocate space for the shadow space
        a.sub(asmjit::x86::rsp, 40);
        // call puts
        a.call(puts);
        // restore shadow space
        a.add(asmjit::x86::rsp, 40);

        jc.pos_last_word = pos;
    }

    // support s" for compiler code generation
    static void genImmediateSQuote()
    {
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;
        std::string word = words[pos];
        jc.word = word;
        printf("genImmediateSQuote: %s\n", word.c_str());
        auto address = stripIndex(word);

        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        commentWithWord(" ; ----- s\" stacking text ");
        a.mov(asmjit::x86::rcx, address);
        pushSS(asmjit::x86::rcx);

        jc.pos_last_word = pos;
    }

    // support s" for interpreter immediate execution.
    static void genTerpImmediateSQuote()
    {
        const auto& words = *jc.words;
        size_t pos = jc.pos_next_word + 1;
        std::string word = words[pos];
        jc.word = word;
        printf("genImmediateSQuote: %s\n", word.c_str());
        auto address = stripIndex(word);
        sm.pushSS(reinterpret_cast<uint64_t>(address));
        jc.pos_last_word = pos;
    }


    //
    static void copyLocalFromDS(asmjit::x86::Gp reg, int offset)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        commentWithWord(" ; ----- pop from stack into ");
        // Pop from the data stack (r15) to the register.
        popDS(reg);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r13, offset), reg);
    }

    static void zeroStackLocation(int offset)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("zeroStackLocation: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        commentWithWord(" ; ----- Clearing ");
        asmjit::x86::Gp zeroReg = asmjit::x86::rcx; //
        a.xor_(zeroReg, zeroReg); // Set zeroReg to zero.
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r13, offset), zeroReg); // Move zero into the stack location.
    }

    // prologue happens when we begin a new word.
    static void genPrologue()
    {
        jc.resetContext();
        if (logging) std::cout << "; gen_prologue\n";
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- function prologue -------------------------");
        a.nop();
        entryFunction();
        a.comment(" ; ----- ENTRY label");


        FunctionEntryExitLabel funcLabels;
        funcLabels.entryLabel = a.newLabel();
        funcLabels.exitLabel = a.newLabel();
        a.bind(funcLabels.entryLabel);


        // Save on loopStack
        const LoopLabel loopLabel{LoopType::FUNCTION_ENTRY_EXIT, funcLabels};
        loopStack.push(loopLabel);

        if (logging) std::cout << " ; gen_prologue: " << static_cast<void*>(jc.assembler) << "\n";
    }


    // happens just before the function returns
    static void genEpilogue()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_epilogue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        jc.epilogueLabel = a.newLabel();
        a.bind(jc.epilogueLabel);

        a.nop();
        a.comment(" ; ----- gen_epilogue");

        a.comment(" ; ----- EXIT label");

        // Check if loopStack is empty
        if (loopStack.empty())
        {
            throw std::runtime_error("gen_epilogue: loopStack is empty");
        }

        auto loopLabelVariant = loopStack.top();
        if (loopLabelVariant.type != LoopType::FUNCTION_ENTRY_EXIT)
        {
            throw std::runtime_error("gen_epilogue: Top of loopStack is not a function entry/exit label");
        }

        const auto& label = std::get<FunctionEntryExitLabel>(loopLabelVariant.label);
        a.bind(label.exitLabel);
        loopStack.pop();

        // locals copy return values to stack.
        const int totalLocalsCount = arguments_to_local_count + locals_count + returned_arguments_count;
        if (totalLocalsCount > 0)
        {
            a.comment(" ; ----- LOCALS in use");

            if (returned_arguments_count > 0)
            {
                a.comment(" ; ----- copy any return values to stack");
                // Copy `returned_arguments_count` values onto the data stack
                for (int i = 0; i < returned_arguments_count; ++i)
                {
                    int offset = (i + arguments_to_local_count + locals_count) * 8;
                    // Offset relative to the stack base.
                    jc.word = findLocalByOffset(offset);

                    commentWithWord(" ; ----- copy return value ");
                    asmjit::x86::Gp returnValueReg = asmjit::x86::ecx;
                    a.mov(returnValueReg, asmjit::x86::qword_ptr(asmjit::x86::r13, offset));
                    // Move the return value from the stack location to the register.
                    pushDS(returnValueReg); // Push the return value onto the data stack (r15).
                }
            }
            // Free the total stack space on the locals stack
            a.comment(" ; ----- free locals");
            a.add(asmjit::x86::r13, totalLocalsCount * 8);
            // Restore the return stack pointer by adding the total local count.
            arguments_to_local_count = locals_count = returned_arguments_count = 0;
        }

        exitFunction();
        // Free the total stack space on the return stack pointer.
        a.ret();
    }


    // exit jump off the word.
    static void genExit()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_exit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_exit");

        // Create a temporary stack to hold the popped labels
        std::stack<LoopLabel> tempStack;
        bool found = false;
        FunctionEntryExitLabel exitLabel;

        // Search for the closest FUNCTION_ENTRY_EXIT label
        while (!loopStack.empty())
        {
            LoopLabel topLabel = loopStack.top();
            loopStack.pop();

            // Check if the current label is of type FUNCTION_ENTRY_EXIT
            if (topLabel.type == LoopType::FUNCTION_ENTRY_EXIT)
            {
                exitLabel = std::get<FunctionEntryExitLabel>(topLabel.label);
                found = true;
                break;
            }

            // Push the label into the temporary stack
            tempStack.push(topLabel);
        }

        // Push back all labels to the loopStack
        while (!tempStack.empty())
        {
            loopStack.push(tempStack.top());
            tempStack.pop();
        }

        // Handle the case when no FUNCTION_ENTRY_EXIT label is found
        if (!found)
        {
            throw std::runtime_error("gen_exit: No FUNCTION_ENTRY_EXIT label found above current context");
        }

        // Bind the exit label
        a.comment(" ; -----Jump to EXIT label");
        a.bind(exitLabel.exitLabel);

        if (logging) std::cout << " ; gen_exit: Bound to function exit label\n";
    }


    // spit out a charachter
    static void genEmit()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_emit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_emit");
        popDS(asmjit::x86::rcx);
        preserveStackPointers();
        // Allocate space for the shadow space
        a.sub(asmjit::x86::rsp, 40);
        a.call(asmjit::imm(reinterpret_cast<void*>(prim_emit)));
        a.add(asmjit::x86::rsp, 40);
        restoreStackPointers();
    }

    static void dotS()
    {
        sm.displayStacks();
    }

    static void words()
    {
        d.list_words();
    }

    static void prim_forget()
    {
        d.forgetLastWord();
    }

    static void genForget()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_emit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_emit");

        preserveStackPointers();
        // Allocate space for the shadow space (32 bytes).
        a.sub(asmjit::x86::rsp, 32);
        a.call(asmjit::imm(reinterpret_cast<void*>(prim_forget)));
        // Restore stack.
        a.add(asmjit::x86::rsp, 32);
        restoreStackPointers();
    }


    // display labels

    static void displayBeginLabel(BeginAgainRepeatUntilLabel* label)
    {
        // display label contents
        printf(" ; ----- BEGIN label\n");
    }


    static void genDot()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_emit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_dot");
        popDS(asmjit::x86::rcx);
        preserveStackPointers();
        // Allocate space for the shadow space (32 bytes).
        a.sub(asmjit::x86::rsp, 32);
        a.call(asmjit::imm(reinterpret_cast<void*>(printDecimal)));
        // Restore stack.
        a.add(asmjit::x86::rsp, 32);
        restoreStackPointers();
    }


    static void prim_depth()
    {
        const uint64_t depth = sm.getDSDepth();
        sm.pushDS(depth + 1);
    }

    static void prim_depth2()
    {
        const uint64_t depth = sm.getDSDepth();
        sm.pushDS(depth);
    }


    static void genDepth()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_emit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_emit");

        preserveStackPointers();
        // Allocate space for the shadow space (32 bytes).
        a.sub(asmjit::x86::rsp, 32);
        a.call(asmjit::imm(reinterpret_cast<void*>(prim_depth)));
        // Restore stack.
        a.add(asmjit::x86::rsp, 32);
        restoreStackPointers();
    }


    static void genDepth2()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_emit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_emit");

        preserveStackPointers();
        // Allocate space for the shadow space (32 bytes).
        a.sub(asmjit::x86::rsp, 32);
        a.call(asmjit::imm(reinterpret_cast<void*>(prim_depth2)));
        // Restore stack.
        a.add(asmjit::x86::rsp, 32);
        restoreStackPointers();
    }

    static void genPushLong()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_push_long: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_push_long");
        a.comment(" ; Push long value onto the stack");
        a.mov(asmjit::x86::rcx, jc.uint64_A);
        pushDS(asmjit::x86::rcx);
    }

    static void genSubLong()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genSubLong: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genSubLong");
        a.comment(" ; Subtract immediate long value from the top of the stack");

        // Pop value from the stack into `rax`
        popDS(asmjit::x86::rax);

        // Subtract immediate value `jc.uint64_A` from `rax`
        a.sub(asmjit::x86::rax, jc.uint64_A);

        // Push the result back onto the stack
        pushDS(asmjit::x86::rax);
    }

    static void genPlusLong()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genPlusLong: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genPlusLong");
        a.comment(" ; Add immediate long value to the top of the stack");

        // Pop value from the stack into `rax`
        popDS(asmjit::x86::rax);

        // Add immediate value `jc.uint64_A` to `rax`
        a.add(asmjit::x86::rax, jc.uint64_A);

        // Push the result back onto the stack
        pushDS(asmjit::x86::rax);
    }

    static ForthFunction endGeneration()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("end: Assembler not initialized");
        }
        // Finalize the function
        ForthFunction func;
        if (const asmjit::Error err = jc.rt.add(&func, &jc.code))
        {
            throw std::runtime_error(asmjit::DebugUtils::errorAsString(err));
        }

        return func;
    }

    // return a function after building a function around its generator fn
    static ForthFunction build_forth(const ForthFunction fn)
    {
        if (logging) std::cout << "; building forth function ... \n";
        if (!jc.assembler)
        {
            throw std::runtime_error("build: Assembler not initialized");
        }
        genPrologue();
        fn();
        genEpilogue();
        const ForthFunction new_func = endGeneration();
        return new_func;
    }

    static void genCall(ForthFunction fn)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_call: Assembler not initialized");
        }
        auto& a = *jc.assembler;

        a.comment(" ; ----- gen_call");
        a.mov(asmjit::x86::rax, fn);
        a.call(asmjit::x86::rax);
    }


    // Executable function pointer


    /*
    static ExecFunc endExec()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("end: Assembler not initialized");
        }
        // Finalize the function
        ExecFunc func;
        if (const asmjit::Error err = jc.rt.add(&func, &jc.code))
        {
            throw std::runtime_error(asmjit::DebugUtils::errorAsString(err));
        }

        return func;
    }


    static void generateExecFunction()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_push_long: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_exec");

        a.mov(asmjit::x86::rax, asmjit::x86::ptr(asmjit::x86::rsp, 8));
        a.call(asmjit::x86::rax);

        a.ret();

        const ExecFunc new_func = endExec();
        exec = new_func;
    }
    */


    // return stack words..

    static void genToR()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_toR: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_toR");

        asmjit::x86::Gp value = asmjit::x86::r8; // Temporary register for value

        // Pop value from DS (represented by r15)
        a.comment(" ; Pop value from DS");
        a.mov(value, asmjit::x86::ptr(asmjit::x86::r15));
        a.add(asmjit::x86::r15, sizeof(uint64_t));

        // Push value to RS (represented by r14)
        a.comment(" ; Push value to RS");
        a.sub(asmjit::x86::r14, sizeof(uint64_t));
        a.mov(asmjit::x86::ptr(asmjit::x86::r14), value);
    }

    static void genRFrom()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_rFrom: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_rFrom");

        asmjit::x86::Gp value = asmjit::x86::r8; // Temporary register for value

        // Pop value from RS (represented by r14)
        a.comment(" ; Pop value from RS");
        a.mov(value, asmjit::x86::ptr(asmjit::x86::r14));
        a.add(asmjit::x86::r14, sizeof(uint64_t));

        // Push value to DS (represented by r15)
        a.comment(" ; Push value to DS");
        a.sub(asmjit::x86::r15, sizeof(uint64_t));
        a.mov(asmjit::x86::ptr(asmjit::x86::r15), value);
    }


    static void genRFetch()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_rFetch: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_rFetch");

        asmjit::x86::Gp value = asmjit::x86::r8; // Temporary register for value

        // Fetch (not pop) value from RS (represented by r14)
        a.comment(" ; Fetch value from RS");
        a.mov(value, asmjit::x86::ptr(asmjit::x86::r14));

        // Push fetched value to DS (represented by r15)
        a.comment(" ; Push fetched value to DS");
        a.sub(asmjit::x86::r15, sizeof(uint64_t));
        a.mov(asmjit::x86::ptr(asmjit::x86::r15), value);
    }


    static void genRPFetch()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_rpFetch: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_rpFetch");

        asmjit::x86::Gp rsPointer = asmjit::x86::r8; // Temporary register for RS pointer

        // Get RS pointer (which is in r14) and push it to DS (r15)
        a.comment(" ; Fetch RS pointer and push to DS");
        a.mov(rsPointer, asmjit::x86::r14);
        a.sub(asmjit::x86::r15, sizeof(uint64_t));
        a.mov(asmjit::x86::ptr(asmjit::x86::r15), rsPointer);
    }


    static void genSPFetch()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_spFetch: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_spFetch");

        asmjit::x86::Gp dsPointer = asmjit::x86::r8; // Temporary register for DS pointer

        // Get DS pointer (which is in r15) and push it to the DS itself
        a.comment(" ; Fetch DS pointer and push to DS");
        a.mov(dsPointer, asmjit::x86::r15);
        a.sub(asmjit::x86::r15, sizeof(uint64_t));
        a.mov(asmjit::x86::ptr(asmjit::x86::r15), dsPointer);
    }


    static void genSPStore()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_spStore: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_spStore");

        asmjit::x86::Gp newDsPointer = asmjit::x86::r8; // Temporary register for new DS pointer

        // Pop new data stack pointer value from the data stack itself
        a.comment(" ; Pop new DS pointer from DS");
        a.mov(newDsPointer, asmjit::x86::ptr(asmjit::x86::r15));
        a.add(asmjit::x86::r15, sizeof(uint64_t));

        // Set DS pointer (r15) to the new data stack pointer value
        a.comment(" ; Store new DS pointer to r15");
        a.mov(asmjit::x86::r15, newDsPointer);
    }


    static void genRPStore()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_rpStore: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_rpStore");

        asmjit::x86::Gp newRsPointer = asmjit::x86::r8; // Temporary register for new RS pointer

        // Pop new return stack pointer value from the data stack
        a.comment(" ; Pop new RS pointer from DS");
        a.mov(newRsPointer, asmjit::x86::ptr(asmjit::x86::r15));
        a.add(asmjit::x86::r15, sizeof(uint64_t));

        // Set RS pointer (r14) to the new return stack pointer value
        a.comment(" ; Store new RS pointer to r14");
        a.mov(asmjit::x86::r14, newRsPointer);
    }

    // store and fetch from memory


    // Generates the Forth @ (fetch) operation
    static void genAT()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genFetch: Assembler not initialized");
        }

        // Fetch the value at the address and push it onto the data stack
        loadFromDS();
    }

    static void genStore()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genStore: Assembler not initialized");
        }

        // Store the value from the data stack into the specified address
        storeFromDS();
    }


    static void genDo()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_push_long: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_do");
        a.nop();
        asmjit::x86::Gp currentIndex = asmjit::x86::rdx; // Current index
        asmjit::x86::Gp limit = asmjit::x86::rcx; // Limit

        // Pop current index and limit from the data stack
        popDS(currentIndex);
        popDS(limit);

        // Push current index and limit onto the return stack
        pushRS(limit);
        pushRS(currentIndex);

        // Increment the DO loop depth counter
        doLoopDepth++;

        // Create labels for loop start and end
        a.nop();
        a.comment(" ; ----- DO label");

        DoLoopLabel doLoopLabel;
        doLoopLabel.doLabel = a.newLabel();
        doLoopLabel.loopLabel = a.newLabel();
        doLoopLabel.leaveLabel = a.newLabel();
        doLoopLabel.hasLeave = false;
        a.bind(doLoopLabel.doLabel);

        // Create a LoopLabel struct and push it onto the unified loopStack
        LoopLabel loopLabel;
        loopLabel.type = LoopType::DO_LOOP;
        loopLabel.label = doLoopLabel;

        loopStack.push(loopLabel);
    }


    static void genLoop()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_loop: Assembler not initialized");
        }

        // check if loopStack is empty
        if (loopStack.empty())
            throw std::runtime_error("gen_loop: loopStack is empty");

        const auto loopLabelVariant = loopStack.top();
        loopStack.pop(); // We are at the end of the loop.

        if (loopLabelVariant.type != LoopType::DO_LOOP)
            throw std::runtime_error("gen_loop: Current loop is not a DO loop");

        const auto& loopLabel = std::get<DoLoopLabel>(loopLabelVariant.label);

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_loop");
        a.nop();

        genLeaveLoopOnEscapeKey(a, loopLabel);

        asmjit::x86::Gp currentIndex = asmjit::x86::rcx; // Current index
        asmjit::x86::Gp limit = asmjit::x86::rdx; // Limit
        a.nop(); // no-op

        a.comment(" ; Pop current index and limit from return stack");
        popRS(currentIndex);
        popRS(limit);
        a.comment(" ; Pop limit back on return stack");
        pushRS(limit);

        a.comment(" ; Increment current index by 1");
        a.add(currentIndex, 1);

        // Push the updated index back onto RS
        a.comment(" ; Push current index back to RS");
        pushRS(currentIndex);

        // Check if current index is less than limit
        a.cmp(currentIndex, limit);

        a.comment(" ; Jump to loop start if still looping.");
        // Jump to loop start if current index is less than the limit

        a.jl(loopLabel.doLabel);

        a.comment(" ; ----- LEAVE and loop label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        // Drop the current index and limit from the return stack
        a.comment(" ; ----- drop loop counters");
        popRS(currentIndex);
        popRS(limit);
        a.nop(); // no-op

        // Decrement the DO loop depth counter
        doLoopDepth--;
    }

    static void genPlusLoop()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_plus_loop: Assembler not initialized");
        }

        // check if loopStack is empty
        if (loopStack.empty())
            throw std::runtime_error("gen_plus_loop: loopStack is empty");

        const auto loopLabelVariant = loopStack.top();
        loopStack.pop(); // We are at the end of the loop.

        if (loopLabelVariant.type != LoopType::DO_LOOP)
            throw std::runtime_error("gen_plus_loop: Current loop is not a DO loop");

        const auto& loopLabel = std::get<DoLoopLabel>(loopLabelVariant.label);

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_plus_loop");

        asmjit::x86::Gp currentIndex = asmjit::x86::rcx; // Current index
        asmjit::x86::Gp limit = asmjit::x86::rdx; // Limit
        asmjit::x86::Gp increment = asmjit::x86::rsi; // Increment value

        genLeaveLoopOnEscapeKey(a, loopLabel);
        a.nop(); // no-op

        // Pop current index and limit from return stack
        a.comment(" ; Pop current index and limit from return stack");
        popRS(currentIndex);
        popRS(limit);
        a.comment(" ; Pop limit back on return stack");
        pushRS(limit);

        // Pop the increment value from data stack
        a.comment(" ; Pop the increment value from data stack");
        popDS(increment);

        // Add increment to current index
        a.comment(" ; Add increment to current index");
        a.add(currentIndex, increment);

        // Push the updated index back onto RS
        a.comment(" ; Push current index back to RS");
        pushRS(currentIndex);

        // Check if current index is less than limit for positive increment
        // or greater than limit for negative increment
        a.comment(" ; Check loop condition based on increment direction");
        a.cmp(increment, 0);
        asmjit::Label positiveIncrement = a.newLabel();
        asmjit::Label loopEnd = a.newLabel();
        a.jg(positiveIncrement);

        // Negative increment
        a.cmp(currentIndex, limit);
        a.jge(loopLabel.doLabel); // Jump if currentIndex >= limit for negative increment
        a.jmp(loopEnd); // Skip positive increment check

        a.comment(" ; ----- Positive increment");
        a.bind(positiveIncrement);
        a.cmp(currentIndex, limit);
        a.jl(loopLabel.doLabel); // Jump if currentIndex < limit for positive increment

        a.comment(" ; ----- LOOP END");
        a.bind(loopEnd);

        a.comment(" ; ----- LEAVE and loop label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        // Drop the current index and limit from the return stack
        a.comment(" ; ----- drop loop counters");
        popRS(currentIndex);
        popRS(limit);
        a.nop(); // no-op

        // Decrement the DO loop depth counter
        doLoopDepth--;
    }


    static void genI()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_I: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_I");

        // Check if there is at least one loop counter on the unified stack
        if (doLoopDepth == 0)
        {
            throw std::runtime_error("gen_I: No matching DO_LOOP structure on the stack");
        }

        // Temporary register to hold the current index
        asmjit::x86::Gp currentIndex = asmjit::x86::rcx;

        // Load the innermost loop index (top of the RS) into currentIndex
        a.comment(" ; Copy top of RS to currentIndex");
        a.mov(currentIndex, asmjit::x86::ptr(asmjit::x86::r14)); // Assuming r14 is used for the RS

        // Push currentIndex onto DS
        a.comment(" ; Push currentIndex onto DS");
        pushDS(currentIndex);
    }


    static void genJ()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_j: Assembler not initialized");
        }

        if (doLoopDepth < 2)
        {
            throw std::runtime_error("gen_j: Not enough nested DO-loops available");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_j");

        asmjit::x86::Gp indexReg = asmjit::x86::rax;

        // Read `J` (which is at depth - 2)
        a.comment(" ; Load index of outer loop (depth - 2) into indexReg");
        a.mov(indexReg, asmjit::x86::ptr(asmjit::x86::r14, 3 * 8)); // Offset for depth - 2 for index

        // Push indexReg onto DS
        a.comment(" ; Push indexReg onto DS");
        pushDS(indexReg);
    }


    static void genK()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_k: Assembler not initialized");
        }

        if (doLoopDepth < 3)
        {
            throw std::runtime_error("gen_k: Not enough nested DO-loops available");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_k");

        asmjit::x86::Gp indexReg = asmjit::x86::rax;

        a.comment(" ; Load index of outermost loop (depth - 3) into indexReg");
        a.mov(indexReg, asmjit::x86::ptr(asmjit::x86::r14, 5 * 8));

        // Push indexReg onto DS
        a.comment(" ; Push indexReg onto DS");
        pushDS(indexReg);
    }


    static void genLeave()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_leave: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_leave");
        a.nop();

        if (loopStack.empty())
        {
            throw std::runtime_error("gen_leave: No loop to leave from");
        }

        // Save current state of loop stack to temp stack
        saveStackToTemp();

        bool found = false;
        asmjit::Label targetLabel;

        std::stack<LoopLabel> workingStack = tempLoopStack;

        // Search for the appropriate leave label in the temporary stack
        while (!workingStack.empty())
        {
            LoopLabel topLabel = workingStack.top();
            workingStack.pop();

            switch (topLabel.type)
            {
            case DO_LOOP:
                {
                    const auto& loopLabel = std::get<DoLoopLabel>(topLabel.label);
                    targetLabel = loopLabel.leaveLabel;
                    found = true;
                    a.comment(" ; Jumps to do loop's leave label");
                    break;
                }
            case BEGIN_AGAIN_REPEAT_UNTIL:
                {
                    const auto& loopLabel = std::get<BeginAgainRepeatUntilLabel>(topLabel.label);
                    targetLabel = loopLabel.leaveLabel;
                    found = true;
                    a.comment(" ; Jumps to begin/again/repeat/until leave label");
                    break;
                }
            default:
                // Continue to look for the correct label
                break;
            }

            if (found)
            {
                break;
            }
        }

        if (!found)
        {
            throw std::runtime_error("gen_leave: No valid loop label found");
        }

        // Reconstitute the temporary stack back into the loopStack
        restoreStackFromTemp();

        // Jump to the found leave label
        a.jmp(targetLabel);
    }


    static void genBegin()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_begin: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_begin");
        a.nop();

        BeginAgainRepeatUntilLabel beginLabel;
        // Create all possible labels here.
        beginLabel.beginLabel = a.newLabel();
        beginLabel.untilLabel = a.newLabel();
        beginLabel.againLabel = a.newLabel();
        beginLabel.whileLabel = a.newLabel();
        beginLabel.leaveLabel = a.newLabel();

        a.comment(" ; LABEL for BEGIN");
        a.bind(beginLabel.beginLabel);

        // Push the new label struct onto the unified stack
        loopStack.push({BEGIN_AGAIN_REPEAT_UNTIL, beginLabel});
    }


    static void genLeaveAgainOnEscapeKey(asmjit::x86::Assembler& a, const BeginAgainRepeatUntilLabel& beginLabels)
    {
        // optionally generate code to check for escape key pressed
        if (jc.optLoopCheck)
        {
            // if escape key pressed leave loop.
            // shadow stack
            a.comment("; -- check for escape key and leave if pressed");
            a.push(asmjit::x86::rax);
            a.sub(asmjit::x86::rsp, 40);
            a.call(escapePressed);
            a.add(asmjit::x86::rsp, 40);
            // compare rax with 0
            a.cmp(asmjit::x86::rax, 0);
            a.pop(asmjit::x86::rax);
            a.comment("; jump to leave label");
            a.jne(beginLabels.leaveLabel);
        }
    }


    static void genLeaveLoopOnEscapeKey(asmjit::x86::Assembler& a, const DoLoopLabel& l)
    {
        // optionally generate code to check for escape key pressed
        if (jc.optLoopCheck)
        {
            // if escape key pressed leave loop.
            // shadow stack
            a.comment("; -- check for escape key and leave if pressed");
            a.push(asmjit::x86::rax);
            a.sub(asmjit::x86::rsp, 40);
            a.call(escapePressed);
            a.add(asmjit::x86::rsp, 40);
            // compare rax with 0
            a.cmp(asmjit::x86::rax, 0);
            a.pop(asmjit::x86::rax);
            a.comment("; jump to leave label");
            a.jne(l.leaveLabel);
        }
    }


    static void genAgain()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_again: Assembler not initialized");
        }

        if (loopStack.empty() || loopStack.top().type != BEGIN_AGAIN_REPEAT_UNTIL)
        {
            throw std::runtime_error("gen_again: No matching BEGIN_AGAIN_REPEAT_UNTIL structure on the stack");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_again");
        a.nop();

        auto beginLabels = std::get<BeginAgainRepeatUntilLabel>(loopStack.top().label);
        loopStack.pop();

        genLeaveAgainOnEscapeKey(a, beginLabels);
        beginLabels.againLabel = a.newLabel();
        a.jmp(beginLabels.beginLabel);

        a.nop();
        a.comment("LABEL for AGAIN");
        a.bind(beginLabels.againLabel);
        a.nop();
        a.comment("LABEL for LEAVE");
        a.bind(beginLabels.leaveLabel);

        a.nop();
        a.comment("LABEL for WHILE");
        a.bind(beginLabels.whileLabel);
    }


    static void genRepeat()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_repeat: Assembler not initialized");
        }

        if (loopStack.empty() || loopStack.top().type != BEGIN_AGAIN_REPEAT_UNTIL)
        {
            throw std::runtime_error("gen_repeat: No matching BEGIN_AGAIN_REPEAT_UNTIL structure on the stack");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_repeat");
        a.nop();

        auto beginLabels = std::get<BeginAgainRepeatUntilLabel>(loopStack.top().label);
        loopStack.pop();

        genLeaveAgainOnEscapeKey(a, beginLabels);
        beginLabels.repeatLabel = a.newLabel();
        a.jmp(beginLabels.beginLabel);
        a.bind(beginLabels.repeatLabel);
        a.nop();
        a.comment("LABEL for LEAVE");
        a.bind(beginLabels.leaveLabel);
        a.comment("LABEL for UNTIL");
        a.nop();
        a.bind(beginLabels.whileLabel);
    }


    static void genUntil()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_until: Assembler not initialized");
        }

        if (loopStack.empty() || loopStack.top().type != BEGIN_AGAIN_REPEAT_UNTIL)
        {
            throw std::runtime_error("gen_until: No matching BEGIN_AGAIN_REPEAT_UNTIL structure on the stack");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_until");
        a.nop();

        // Get the label from the unified stack
        const auto& beginLabels = std::get<BeginAgainRepeatUntilLabel>(loopStack.top().label);

        asmjit::x86::Gp topOfStack = asmjit::x86::rax;
        popDS(topOfStack);
        genLeaveAgainOnEscapeKey(a, beginLabels);
        a.comment(" ; Jump back to beginLabel if top of stack is zero");
        a.test(topOfStack, topOfStack);
        a.jz(beginLabels.beginLabel);

        a.comment("LABEL for UNTIL");
        // Bind the appropriate labels
        a.bind(beginLabels.untilLabel);
        a.nop();
        a.comment("LABEL for LEAVE");
        a.bind(beginLabels.leaveLabel);

        // Pop the stack element as we're done with this construct
        loopStack.pop();
    }


    static void genWhile()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_while: Assembler not initialized");
        }

        if (loopStack.empty() || loopStack.top().type != BEGIN_AGAIN_REPEAT_UNTIL)
        {
            throw std::runtime_error("gen_while: No matching BEGIN_AGAIN_REPEAT_UNTIL structure on the stack");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_while");
        a.nop();

        auto beginLabel = std::get<BeginAgainRepeatUntilLabel>(loopStack.top().label);
        asmjit::x86::Gp topOfStack = asmjit::x86::rax;
        popDS(topOfStack);

        a.comment(" ; Conditional jump to whileLabel if top of stack is zero");
        a.test(topOfStack, topOfStack);
        a.comment("Jump to WHILE if zero");
        a.jz(beginLabel.whileLabel);
        a.nop();
    }

    static void genIf()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genIf: Assembler not initialized");
        }

        auto& a = *jc.assembler;

        IfThenElseLabel branches;
        branches.ifLabel = a.newLabel();
        branches.elseLabel = a.newLabel();
        branches.thenLabel = a.newLabel();
        branches.leaveLabel = a.newLabel();
        branches.exitLabel = a.newLabel();
        branches.hasElse = false;
        branches.hasLeave = false;
        branches.hasExit = false;

        // Push the new IfThenElseLabel structure onto the unified loopStack
        loopStack.push({IF_THEN_ELSE, branches});

        a.comment(" ; ----- gen_if");
        a.nop();

        // Pop the condition flag from the data stack
        asmjit::x86::Gp flag = asmjit::x86::rax;
        popDS(flag);

        // Conditional jump to either the ELSE or THEN location
        a.test(flag, flag);
        a.jz(branches.ifLabel);
    }


    static void genElse()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genElse: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_else");
        a.nop();

        if (!loopStack.empty() && loopStack.top().type == IF_THEN_ELSE)
        {
            auto branches = std::get<IfThenElseLabel>(loopStack.top().label);
            a.comment(" ; jump past else block");
            a.jmp(branches.elseLabel); // Jump to the code after the ELSE block
            a.comment(" ; ----- label for ELSE");
            a.bind(branches.ifLabel);
            branches.hasElse = true;

            // Update the stack with the modified branches
            loopStack.pop();
            loopStack.push({IF_THEN_ELSE, branches});
        }
        else
        {
            throw std::runtime_error("genElse: No matching IF_THEN_ELSE structure on the stack");
        }
    }

    static void genThen()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genThen: Assembler not initialized");
        }

        auto& a = *jc.assembler;

        if (!loopStack.empty() && loopStack.top().type == IF_THEN_ELSE)
        {
            auto branches = std::get<IfThenElseLabel>(loopStack.top().label);
            if (branches.hasElse)
            {
                a.bind(branches.elseLabel); // Bind the ELSE label
            }
            else if (branches.hasLeave)
            {
                a.bind(branches.leaveLabel); // Bind the leave label
            }
            else if (branches.hasExit)
            {
                a.bind(branches.exitLabel); // Bind the exit label
            }
            else
            {
                a.bind(branches.ifLabel);
            }
            loopStack.pop();
        }
        else
        {
            throw std::runtime_error("genThen: No matching IF_THEN_ELSE structure on the stack");
        }
    }


    static void genSub()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genSub: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genSub");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values from the stack, subtract the first from the second, and push the result

        a.comment(" ; Pop two values from the stack");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.sub(asmjit::x86::qword_ptr(ds), firstVal); // Subtract first value from second value
    }


    static void genPlus()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genPlus: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genPlus");

        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp secondVal = asmjit::x86::rbx;

        a.comment(" ; Add two values from the stack");
        popDS(firstVal);
        popDS(secondVal);
        a.add(secondVal, firstVal); // Add first value to second value
        pushDS(secondVal);
    }

    static void genDiv()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDiv: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDiv");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp dividend = asmjit::x86::rax;
        asmjit::x86::Gp divisor = asmjit::x86::rcx;

        // Pop two values from the stack, divide the first by the second, and push the quotient

        a.comment(" ; Perform / division");
        a.mov(divisor, asmjit::x86::qword_ptr(ds)); // Load the second value (divisor)
        a.add(ds, 8); // Adjust stack pointer

        a.mov(dividend, asmjit::x86::qword_ptr(ds)); // Load the first value (dividend)
        a.add(ds, 8); // Adjust stack pointer

        a.mov(asmjit::x86::rdx, 0); // Clear RDX for unsigned division
        // RAX already contains the dividend (first value)

        a.idiv(divisor); // Perform signed division: RDX:RAX / divisor

        a.sub(ds, 8); // Adjust stack pointer back
        a.mov(asmjit::x86::qword_ptr(ds), asmjit::x86::rax); // Store quotient back on stack
    }

    static void genMul()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genMul: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genMul");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp secondVal = asmjit::x86::rdx;

        // Pop two values from the stack, multiply them, and push the result

        a.comment(" ; * -multiply two values from the stack");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.mov(secondVal, asmjit::x86::qword_ptr(ds)); // Load second value

        a.imul(firstVal, secondVal); // Multiply first and second value, result in firstVal

        a.comment(" ; Push the result onto the stack");
        a.mov(asmjit::x86::qword_ptr(ds), firstVal); // Store result back on stack
    }


    // comparisons

    static void genEq()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genEq: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genEq");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; Pop two values compare them and push the result (0 or -1)");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.cmp(asmjit::x86::qword_ptr(ds), firstVal); // Compare with second value
        a.sete(asmjit::x86::al); // Set AL to 1 if equal

        a.neg(asmjit::x86::al); // Set AL to -1 if true, 0 if false
        a.movsx(asmjit::x86::rax, asmjit::x86::al); // Sign-extend AL to RAX (-1 or 0)
        a.mov(asmjit::x86::qword_ptr(ds), asmjit::x86::rax); // Store result on stack
    }

    static void genLt()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genLt: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genLt");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp secondVal = asmjit::x86::rdx;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; < compare them and push the result (0 or -1)");

        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.mov(secondVal, asmjit::x86::qword_ptr(ds)); // Load second value
        a.add(ds, 8); // Adjust stack pointer

        a.cmp(secondVal, firstVal); // Compare second value to first value
        a.setl(asmjit::x86::al); // Set AL to 1 if secondVal < firstVal

        a.movzx(asmjit::x86::rax, asmjit::x86::al); // Zero-extend AL to RAX
        a.neg(asmjit::x86::rax); // Set RAX to -1 if true (1 -> -1), 0 remains 0

        a.sub(ds, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(ds), asmjit::x86::rax); // Store result on stack
    }

    static void genGt()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genGt: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genGt");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; > compare them and push the result (0 or -1)");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.cmp(asmjit::x86::qword_ptr(ds), firstVal); // Compare with second value
        a.setg(asmjit::x86::al); // Set AL to 1 if greater

        a.neg(asmjit::x86::al); // Set AL to -1 if true, 0 if false
        a.movsx(asmjit::x86::rax, asmjit::x86::al); // Sign-extend AL to RAX (-1 or 0)
        a.mov(asmjit::x86::qword_ptr(ds), asmjit::x86::rax); // Store result on stack
    }

    static void genNot()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genNot: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genNot");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;

        // Load the value from the stack
        a.mov(asmjit::x86::rax, asmjit::x86::qword_ptr(ds)); // Load value

        // Use the `NOT` operation to flip the bits
        a.not_(asmjit::x86::rax); // Perform NOT operation

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(ds), asmjit::x86::rax); // Store result back on stack
    }

    static void genAnd()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genAnd: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genAnd");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform AND, and push the result
        a.comment(" ; AND two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.and_(firstVal, asmjit::x86::qword_ptr(ds)); // Perform AND with second value

        a.mov(asmjit::x86::qword_ptr(ds), firstVal); // Store result back on stack
    }


    static void genOR()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genOR: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genOR");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform OR, and push the result
        a.comment(" ; OR two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.or_(firstVal, asmjit::x86::qword_ptr(ds)); // Perform OR with second value

        a.mov(asmjit::x86::qword_ptr(ds), firstVal); // Store result back on stack
    }


    static void genXOR()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genXOR: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genXOR");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform XOR, and push the result
        a.comment(" ; XOR two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(ds)); // Load first value
        a.add(ds, 8); // Adjust stack pointer

        a.xor_(firstVal, asmjit::x86::qword_ptr(ds)); // Perform XOR with second value

        a.mov(asmjit::x86::qword_ptr(ds), firstVal); // Store result back on stack
    }

    // stack ops

    static void genDSAT()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDSAT: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDSAT");

        asmjit::x86::Gp ds = asmjit::x86::r15; // stack pointer in r15
        asmjit::x86::Gp tempReg = asmjit::x86::rax; // temporary register to hold the stack pointer value

        // Move the stack pointer to the temporary register
        a.comment(" ; SP@ - Get the stack pointer value");
        a.mov(tempReg, ds);

        // Push the stack pointer value onto the data stack
        a.comment(" ; Push the stack pointer value onto the data stack");
        pushDS(tempReg);

        a.comment(" ; ----- end of genDSAT");
    }

    static void genDrop()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDrop: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDrop");

        // Assuming r15 is the stack pointer
        a.comment(" ; drop top value");
        asmjit::x86::Gp ds = asmjit::x86::r15;
        a.add(ds, 8); // Adjust stack pointer to drop the top value
    }


    static void genDup()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDup: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDup");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp topValue = asmjit::x86::rax;

        // Duplicate the top value on the stack
        a.comment(" ; Duplicate the top value on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(ds)); // Load top value
        a.sub(ds, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(ds), topValue); // Push duplicated value
    }

    static void genSwap()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genSwap: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genSwap");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp topValue = asmjit::x86::rax;
        asmjit::x86::Gp secondValue = asmjit::x86::rcx;

        // Swap the top two values on the stack
        a.comment(" ; Swap top two values on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(ds)); // Load top value
        a.mov(secondValue, asmjit::x86::qword_ptr(ds, 8)); // Load second value
        a.mov(asmjit::x86::qword_ptr(ds), secondValue); // Store second value in place of top value
        a.mov(asmjit::x86::qword_ptr(ds, 8), topValue); // Store top value in place of second value
    }

    static void genRot()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genRot: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genRot");

        asmjit::x86::Gp ds = asmjit::x86::r15; // stack pointer in r15
        asmjit::x86::Gp topValue = asmjit::x86::rax; // top value in rax
        asmjit::x86::Gp secondValue = asmjit::x86::rcx; // second value in rcx
        asmjit::x86::Gp thirdValue = asmjit::x86::rdx; // third value in rdx

        // Rotate the top three values on the stack
        a.comment(" ; Rotate top three values on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(ds)); // Load top value (TOS)
        a.mov(secondValue, asmjit::x86::qword_ptr(ds, 8)); // Load second value (NOS)
        a.mov(thirdValue, asmjit::x86::qword_ptr(ds, 16)); // Load third value (TOS+2)
        a.mov(asmjit::x86::qword_ptr(ds), thirdValue);
        // Store third value in place of top value (TOS = TOS+2)
        a.mov(asmjit::x86::qword_ptr(ds, 16), secondValue);
        // Store second value into third position (TOS+2 = NOS)
        a.mov(asmjit::x86::qword_ptr(ds, 8), topValue); // Store top value into second position (NOS = TOS)
        a.comment(" ; ----- end of genRot");
    }

    static void genOver()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genOver: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genOver");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp secondValue = asmjit::x86::rax;

        // Duplicate the second value on the stack
        a.comment(" ; Duplicate the second value on the stack");
        a.mov(secondValue, asmjit::x86::qword_ptr(ds, 8)); // Load second value
        a.sub(ds, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(ds), secondValue); // Push duplicated value
    }


    static void genTuck()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genTuck: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genTuck");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp topValue = asmjit::x86::rax;
        asmjit::x86::Gp secondValue = asmjit::x86::rcx;

        // Tuck the top value under the second value on the stack
        a.comment(" ; Tuck the top value under the second value on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(ds)); // Load top value
        a.mov(secondValue, asmjit::x86::qword_ptr(ds, 8)); // Load second value
        a.sub(ds, 8); // Adjust stack pointer to make space
        a.mov(asmjit::x86::qword_ptr(ds), topValue); // Push top value
        a.mov(asmjit::x86::qword_ptr(ds, 8), secondValue); // Access memory at ds + 8
        a.mov(asmjit::x86::qword_ptr(ds, 16), topValue); // Access memory at ds + 16
    }


    static void genNip()
    {
        // generate forth NIP stack word
        if (!jc.assembler)
        {
            throw std::runtime_error("genNip: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genNip");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp topValue = asmjit::x86::rax;

        // NIP's operation is to remove the second item on the stack
        a.comment(" ; Remove the second item from the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(ds)); // Load top value
        a.add(ds, 8); // Adjust stack pointer to skip the second item
        a.mov(asmjit::x86::qword_ptr(ds, 0), topValue); // Move top value to the new top position
    }

    static void genPick(int n)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genPick: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genPick");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;
        asmjit::x86::Gp value = asmjit::x86::rax;

        // Pick the nth value from the stack
        a.comment(" ; Pick the nth value from the stack");

        // Calculate the address offset for the nth element
        int offset = n * 8;

        a.mov(value, asmjit::x86::qword_ptr(ds, offset)); // Load the nth value
        a.sub(ds, 8); // Adjust stack pointer for pushing value
        a.mov(asmjit::x86::qword_ptr(ds), value); // Push the picked value onto the stack
    }


    // Helper function to generate code for pushing constants onto the stack

    static void genPushConstant(int64_t value)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp ds = asmjit::x86::r15; // Stack pointer register

        if (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max())
        {
            // Push a 32-bit immediate value (optimized for smaller constants)
            a.sub(ds, 8); // Reserve space on the stack
            a.mov(asmjit::x86::qword_ptr(ds), static_cast<int32_t>(value));
        }
        else
        {
            // For larger/negative 64-bit constants
            asmjit::x86::Mem stackMem(ds, -8); // Memory location for pushing the constant
            a.sub(ds, 8); // Reserve space on the stack
            a.mov(stackMem, value); // Move the 64-bit value directly to the reserved space
        }
    }

    // Macros or inline functions to call the helper function with specific values
#define GEN_PUSH_CONSTANT_FN(name, value) \
    static void name()                     \
    {                                      \
        genPushConstant(value);            \
    }

    // Define specific constant push functions
    GEN_PUSH_CONSTANT_FN(push1, 1)
    GEN_PUSH_CONSTANT_FN(push2, 2)
    GEN_PUSH_CONSTANT_FN(push3, 3)
    GEN_PUSH_CONSTANT_FN(push4, 4)
    GEN_PUSH_CONSTANT_FN(push8, 8)
    GEN_PUSH_CONSTANT_FN(push16, 16)
    GEN_PUSH_CONSTANT_FN(push32, 32)
    GEN_PUSH_CONSTANT_FN(push64, 64)
    GEN_PUSH_CONSTANT_FN(pushNeg1, -1)
    GEN_PUSH_CONSTANT_FN(SPBASE, sm.getDStop())


    static void gen1Inc()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen1inc: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen1inc - use inc instruction");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;

        // Increment the value at the memory location pointed to by r15
        a.inc(asmjit::x86::qword_ptr(ds));
    }


    static void gen1Dec()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen1inc: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen1inc - use dec instruction");

        // Assuming r15 is the stack pointer
        asmjit::x86::Gp ds = asmjit::x86::r15;

        // Decrement the value at the memory location pointed to by r15
        a.dec(asmjit::x86::qword_ptr(ds));
    }


#define GEN_INC_DEC_FN(name, operation, value) \
    static void name()                         \
    {                                          \
    jc.uint64_A = value;                   \
    operation();                           \
    }

    // Define specific increment functions
    GEN_INC_DEC_FN(gen2Inc, genPlusLong, 2)
    GEN_INC_DEC_FN(gen16Inc, genPlusLong, 16)
    // Define specific decrement functions

    GEN_INC_DEC_FN(gen2Dec, genSubLong, 2)
    GEN_INC_DEC_FN(gen16Dec, genSubLong, 16)

    static void genMulBy10()
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp ds = asmjit::x86::r15; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value
        asmjit::x86::Gp tempResult = asmjit::x86::rdx; // Temporary register for intermediate result

        a.comment("; multiply by ten");
        // Load the top stack value into tempValue
        popDS(tempValue);

        // Perform the shift left by 3 (Value * 8)
        a.mov(tempResult, tempValue);
        a.shl(tempResult, 3);

        // Perform the shift left by 1 (Value * 2)
        a.shl(tempValue, 1);

        // Add the two shifted values
        a.add(tempResult, tempValue);

        // Store the result back on the stack
        pushDS(tempResult);
    }


    // Helper function for left shifts
    static void genLeftShift(int shiftAmount)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp ds = asmjit::x86::r15; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value

        // Load the top stack value into tempValue
        a.mov(tempValue, asmjit::x86::qword_ptr(ds));

        // Perform the shift
        a.shl(tempValue, shiftAmount);

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(ds), tempValue);
    }

    // Helper function for right shifts
    static void genRightShift(int shiftAmount)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp ds = asmjit::x86::r15; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value

        // Load the top stack value into tempValue
        a.mov(tempValue, asmjit::x86::qword_ptr(ds));

        // Perform the shift
        a.shr(tempValue, shiftAmount);

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(ds), tempValue);
    }

    // Macros to define the shift operations
#define GEN_SHIFT_FN(name, shiftAction, shiftAmount) \
    static void name()                               \
    {                                                \
        shiftAction(shiftAmount);                    \
    }

    // Define specific shift functions for multiplication
    GEN_SHIFT_FN(gen2mul, genLeftShift, 1)
    GEN_SHIFT_FN(gen4mul, genLeftShift, 2)
    GEN_SHIFT_FN(gen8mul, genLeftShift, 3)
    GEN_SHIFT_FN(gen16mul, genLeftShift, 4)

    // Define specific shift functions for division
    GEN_SHIFT_FN(gen2Div, genRightShift, 1)
    GEN_SHIFT_FN(gen4Div, genRightShift, 2)
    GEN_SHIFT_FN(gen8Div, genRightShift, 3)

private
:
    // Private constructor to prevent instantiation
    JitGenerator() = default;
    // Private destructor if needed
    ~JitGenerator() = default;
};

#endif //JITGENERATOR_H
