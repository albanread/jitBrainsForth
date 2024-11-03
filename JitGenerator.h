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


// labels for control flow.


// Structure to keep track IF .. ELSE .. LEAVE/EXIT .. THEN
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

inline std::stack<IfThenElseLabel> branchStack;

struct FunctionEntryExitLabel
{
    asmjit::Label entryLabel;
    asmjit::Label exitLabel;
};

inline std::stack<FunctionEntryExitLabel> functionsStack;

struct DoLoopLabel
{
    asmjit::Label doLabel;
    asmjit::Label loopLabel;
    asmjit::Label leaveLabel;
    bool hasLeave;
};

inline std::stack<DoLoopLabel> doLoopStack;

struct BeginAgainRepeatUntilLabel
{
    asmjit::Label beginLabel; // Label for BEGIN
    asmjit::Label againLabel; // Label for AGAIN
    asmjit::Label repeatLabel; // Label for REPEAT
    asmjit::Label untilLabel; // Label for UNTIL
    asmjit::Label whileLabel; // Label for WHILE
    asmjit::Label leaveLabel; // Label for LEAVE
    bool hasAgain; // Flag if AGAIN is present
    bool hasRepeat; // Flag if REPEAT is present
    bool hasUntil; // Flag if UNTIL is present
    bool hasWhile; // Flag if WHILE is present
    bool hasLeave; // Flag if LEAVE is present
};

// Stack to manage the BEGIN..AGAIN REPEAT..UNTIL BEGIN WHILE AGAIN loop structures
inline std::stack<BeginAgainRepeatUntilLabel> beginAgainRepeatUntilStack;

inline JitContext& jc = JitContext::getInstance();
inline ForthDictionary& d = ForthDictionary::getInstance();
inline StackManager& sm = StackManager::getInstance();

inline extern void prim_emit(const uint64_t a)
{
    printf("; prim_emit: %d\n", a);
    putchar(a);
}

static bool logging = false;


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

    static void genOr();

    // Delete copy constructor and assignment operator to prevent copies
    JitGenerator(const JitGenerator&) = delete;
    JitGenerator& operator=(const JitGenerator&) = delete;


    static void entryFunction()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- entryFunction");
        a.nop();
        a.comment("; load datastack to R11");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.dsPtr));
        a.mov(asmjit::x86::r11, asmjit::x86::qword_ptr(asmjit::x86::rax));
        a.comment("; load return stack to R10");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.rsPtr));
        a.mov(asmjit::x86::r10, asmjit::x86::qword_ptr(asmjit::x86::rax));
        a.comment("; load local stack to R9");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.lsPtr));
        a.mov(asmjit::x86::r9, asmjit::x86::qword_ptr(asmjit::x86::rax));
    }

    static void exitFunction()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;

        a.comment(" ; ----- exitFunction");
        a.nop();
        a.comment("; save datastack from R11");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.dsPtr));
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r11);
        a.comment("; save return stack from R10");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.rsPtr));
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r10);
        a.comment("; save local stack from R9");
        a.mov(asmjit::x86::rax, reinterpret_cast<uint64_t>(&sm.lsPtr));
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::rax), asmjit::x86::r9);
    }

    // preserve stack pointers
    static void preserveStackPointers()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        a.comment(" ; preserve r9, r10, r11, stack pointers on dsp");
        // preserve R9, R10, R11 on dsp
        a.push(asmjit::x86::r9);
        a.push(asmjit::x86::r10);
        a.push(asmjit::x86::r11);
    }


    static void restoreStackPointers()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }
        auto& a = *jc.assembler;
        a.comment(" ; restore r9, r10, r11, stack pointers from dsp");
        // restore R9, R10, R11 on dsp
        a.pop(asmjit::x86::r11);
        a.pop(asmjit::x86::r10);
        a.pop(asmjit::x86::r9);
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
        a.comment(" ; save value to the data stack (R11)");
        a.nop();
        a.sub(asmjit::x86::r11, 8);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r11), reg);
    }

    static void popDS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- popDS");
        a.comment(" ; fetch value from the data stack (R11)");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r11));
        a.add(asmjit::x86::r11, 8);
    }

    static void pushRS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- pushRS");
        a.comment(" ; save value to the return stack (R10)");
        a.nop();
        a.sub(asmjit::x86::r10, 8);
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r10), reg);
    }

    static void popRS(asmjit::x86::Gp reg)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- popRS");
        a.comment(" ; fetch value from the return stack (R10)");
        a.nop();
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r10));
        a.add(asmjit::x86::r10, 8);
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
        a.mov(reg, asmjit::x86::qword_ptr(asmjit::x86::r9, offset));
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
        a.mov(asmjit::x86::qword_ptr(asmjit::x86::r9, offset), reg);
    }

    static void allocateLocals(int count)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("entryFunction: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.sub(asmjit::x86::r9, count * 8);
    }

    static void exec(void (*func)())
    {
        __asm__ __volatile__ (
            "push %%rbp;" // Save base pointer
            "mov %%rsp, %%rbp;" // Set base pointer to top of stack
            "push %%rbx;" // Save rbx register
            "push %%rcx;" // Save rcx register
            "push %%rdx;" // Save rdx register
            "push %%rsi;" // Save rsi register
            "push %%rdi;" // Save rdi register
            "push %%r8;" // Save r8 register
            "push %%r9;" // Save r9 register
            "push %%r10;" // Save r10 register
            "push %%r11;" // Save r11 register
            "push %%r12;" // Save r12 register
            "push %%r13;" // Save r13 register

            "mov %0, %%rax;" // Move function pointer to rax
            "call *%%rax;" // Call the function

            "pop %%r13;" // Restore r13 register
            "pop %%r12;" // Restore r12 register
            "pop %%r11;" // Restore r11 register
            "pop %%r10;" // Restore r10 register
            "pop %%r9;" // Restore r9 register
            "pop %%r8;" // Restore r8 register
            "pop %%rdi;" // Restore rdi register
            "pop %%rsi;" // Restore rsi register
            "pop %%rdx;" // Restore rdx register
            "pop %%rcx;" // Restore rcx register
            "pop %%rbx;" // Restore rbx register
            "leave;" // Restore base pointer
            "ret;" // Return

            :
            : "r" (func)
            : "rax"
        );
    }

    // Generator methods
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
        FunctionEntryExitLabel label;
        label.entryLabel = a.newLabel();
        a.bind(label.entryLabel);
        label.exitLabel = a.newLabel();
        // save on functionsStack
        functionsStack.push(label);


        if (logging) printf(" ; gen_prologue: %p\n", jc.assembler);
    }

    static void genEpilogue()
    {
        ;
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_epilogue: Assembler not initialized");
        }

        auto& a = *jc.assembler;

        a.nop();
        a.comment(" ; ----- gen_epilogue");

        a.comment(" ; ----- EXIT label");

        // check if functionsStack is empty
        if (functionsStack.empty())
        {
            throw std::runtime_error("gen_epilogue: functionsStack is empty");
        }

        auto label = functionsStack.top();
        a.bind(label.exitLabel);
        functionsStack.pop();


        exitFunction();
        a.ret();
    }


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
        // Allocate space for the shadow space (32 bytes).
        a.sub(asmjit::x86::rsp, 32);
        a.call(asmjit::imm(reinterpret_cast<void*>(putchar)));
        // Restore stack.
        a.add(asmjit::x86::rsp, 32);
        restoreStackPointers();
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

    static ForthFunction end()
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
        const ForthFunction new_func = end();
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

        // current index on top
        popDS(currentIndex);
        popDS(limit);

        // Push current index and limit onto the return stack
        pushRS(limit);
        pushRS(currentIndex); // push current index

        // Create labels for loop start and end
        a.nop();
        a.comment(" ; ----- DO label");

        DoLoopLabel doLoopLabel;
        doLoopLabel.doLabel = a.newLabel();
        doLoopLabel.loopLabel = a.newLabel();
        doLoopLabel.leaveLabel = a.newLabel();
        doLoopLabel.hasLeave = false;
        a.bind(doLoopLabel.doLabel);
        // push loop label to doLoopStack
        doLoopStack.push(doLoopLabel);
    }

    static void genLoop()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_loop: Assembler not initialized");
        }

        // check if doLoopStack is empty
        if (doLoopStack.empty())
            throw std::runtime_error("gen_loop: doLoopStack is empty");

        const auto loopLabel = doLoopStack.top();
        doLoopStack.pop(); // we are the end of the loop.


        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_loop");
        a.nop();


        asmjit::x86::Gp currentIndex = asmjit::x86::rcx; // Current index
        asmjit::x86::Gp limit = asmjit::x86::rdx; // Limit
        a.nop(); // no-o

        a.comment(" ; Pop current index and limit from return stack");
        popRS(currentIndex);
        popRS(limit);
        a.comment(" ; Pop limit back on return stack");
        pushRS(limit);

        a.comment(" ; Increment current index by 1");
        a.add(currentIndex, 1);

        // Push updated values back onto RS
        a.comment(" ; Push current index back to RS");

        pushRS(currentIndex);

        // check if current index is less than limit
        a.cmp(currentIndex, limit);

        a.comment(" ; Jump to loop start if still looping.");
        a.jl(loopLabel.doLabel);
        // jump to loop start if current index is less than or equal to limit

        a.comment(" ; ----- LEAVE and loop label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        // drop the current index and limit from the return stack
        a.comment(" ; ----- drop loop counters");
        popRS(currentIndex);
        popRS(limit);
        a.nop(); // no-op
    }


    static void genPlusLoop()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_plus_loop: Assembler not initialized");
        }

        // check if doLoopStack is empty
        if (doLoopStack.empty())
            throw std::runtime_error("gen_plus_loop: doLoopStack is empty");

        auto loopLabel = doLoopStack.top();
        doLoopStack.pop(); // we are the end of the loop.


        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_plus_loop");
        a.nop();


        // Define registers to use
        asmjit::x86::Gp currentIndex = asmjit::x86::rcx; // Current index
        asmjit::x86::Gp limit = asmjit::x86::rdx; // Limit
        asmjit::x86::Gp increment = asmjit::x86::rsi; // Increment value

        a.nop();

        // Pop current index and limit from return stack
        popRS(currentIndex);
        popRS(limit);

        // Pop the increment value from data stack
        popDS(increment);

        // Add increment to current index
        a.add(currentIndex, increment);

        // Compare updated current index to limit
        a.cmp(currentIndex, limit);

        // Jump if current index is less than or equal to limit
        a.jl(loopLabel.doLabel);

        a.comment(" ; ----- Loop and Leave label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        a.nop(); // no-op

        // Clean up loop stack
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

        // check if doLoopStack is empty
        if (doLoopStack.empty())
            throw std::runtime_error("gen_plus_loop: doLoopStack is empty");

        const auto& loopLabel = doLoopStack.top();

        // Jump to the leave label
        // the code at leave tidies up the return stack.
        a.comment(" ; Jumps to the leave label");
        a.jmp(loopLabel.leaveLabel);
    }

    static void genI()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_I: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_I");
        a.nop();

        // Check if there is at least one loop counter on the return stack
        if (doLoopStack.empty())
            throw std::runtime_error("gen_I: doLoopStack is empty");

        // Move the top element (current index) from RS to DS
        // Assume R10 is RS and R11 is DS
        asmjit::x86::Gp currentIndex = asmjit::x86::rsi; // Temporary register
        a.comment(" ; Copy top of RS to currentIndex");
        a.mov(currentIndex, asmjit::x86::ptr(asmjit::x86::r10));

        a.comment(" ; Push currentIndex onto DS");
        pushDS(currentIndex);
    }

    static void genJ()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_J: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_J");
        a.nop();

        // Check if there are at least two loop counters on the return stack
        if (doLoopStack.size() < 2)
            throw std::runtime_error("gen_J: Not enough nested loops for J");

        // Move the third element (outer current index) from RS to DS
        // Assume R10 is RS and R11 is DS, and RS is full descending
        asmjit::x86::Gp outerCurrentIndex = asmjit::x86::rsi; // Temporary register
        a.comment(" ; Copy third element of RS to outerCurrentIndex");
        a.mov(outerCurrentIndex, asmjit::x86::ptr(asmjit::x86::r10, 2 * sizeof(uint64_t)));

        a.comment(" ; Push outerCurrentIndex onto DS");
        pushDS(outerCurrentIndex);
    }

    static void genK()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_K: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_K");
        a.nop();

        // Check if there are at least three loop counters on the return stack
        if (doLoopStack.size() < 3)
            throw std::runtime_error("gen_K: Not enough nested loops for K");

        // Move the fifth element (outermost current index) from RS to DS
        // Assume R10 is RS and R11 is DS, and RS is full descending
        asmjit::x86::Gp outermostCurrentIndex = asmjit::x86::rsi; // Temporary register
        a.comment(" ; Copy fifth element of RS to outermostCurrentIndex");
        a.mov(outermostCurrentIndex, asmjit::x86::ptr(asmjit::x86::r10, 4 * sizeof(uint64_t)));

        a.comment(" ; Push outermostCurrentIndex onto DS");
        pushDS(outermostCurrentIndex);
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
        beginLabel.beginLabel = a.newLabel();
        beginLabel.hasAgain = false;
        beginLabel.hasRepeat = false;
        beginLabel.hasUntil = false;
        beginLabel.hasWhile = false;
        beginLabel.hasLeave = false;

        a.comment(" ; LABEL for BEGIN");
        a.bind(beginLabel.beginLabel);
        beginAgainRepeatUntilStack.push(beginLabel);
    }

    static void genAgain()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_again: Assembler not initialized");
        }

        if (beginAgainRepeatUntilStack.empty())
        {
            throw std::runtime_error("gen_again: beginAgainRepeatUntilStack is empty");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_again");
        a.nop();

        auto beginLabel = beginAgainRepeatUntilStack.top();
        beginAgainRepeatUntilStack.pop();

        beginLabel.againLabel = a.newLabel();
        beginLabel.hasAgain = true;
        a.jmp(beginLabel.beginLabel);
        a.bind(beginLabel.againLabel);
        a.bind(beginLabel.whileLabel);
    }

    static void genRepeat()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_repeat: Assembler not initialized");
        }

        if (beginAgainRepeatUntilStack.empty())
        {
            throw std::runtime_error("gen_repeat: beginAgainRepeatUntilStack is empty");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_repeat");
        a.nop();

        auto beginLabel = beginAgainRepeatUntilStack.top();
        beginAgainRepeatUntilStack.pop();

        beginLabel.repeatLabel = a.newLabel();
        beginLabel.hasRepeat = true;
        a.jmp(beginLabel.beginLabel);
        a.bind(beginLabel.repeatLabel);
        a.bind(beginLabel.whileLabel);
    }

    static void genUntil()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_until: Assembler not initialized");
        }

        if (beginAgainRepeatUntilStack.empty())
        {
            throw std::runtime_error("gen_until: beginAgainRepeatUntilStack is empty");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_until");
        a.nop();

        auto beginLabel = beginAgainRepeatUntilStack.top();
        beginAgainRepeatUntilStack.pop();

        asmjit::x86::Gp topOfStack = asmjit::x86::rax;

        popDS(topOfStack);
        a.comment(" ; Jump back to beginLabel if top of stack is zero");
        a.test(topOfStack, topOfStack);
        a.jz(beginLabel.beginLabel);

        beginLabel.untilLabel = a.newLabel();
        beginLabel.hasUntil = true;
        a.bind(beginLabel.untilLabel);
    }

    static void genWhile()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_while: Assembler not initialized");
        }

        if (beginAgainRepeatUntilStack.empty())
        {
            throw std::runtime_error("gen_while: beginAgainRepeatUntilStack is empty");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_while");
        a.nop();

        auto beginLabel = beginAgainRepeatUntilStack.top();
        beginLabel.whileLabel = a.newLabel();
        beginLabel.hasWhile = true;

        asmjit::x86::Gp topOfStack = asmjit::x86::rax;

        popDS(topOfStack);
        a.comment(" ; Conditional jump to whileLabel if top of stack is zero");
        a.test(topOfStack, topOfStack);
        a.jz(beginLabel.whileLabel);

        beginAgainRepeatUntilStack.pop();
        beginAgainRepeatUntilStack.push(beginLabel);
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
        branchStack.push(branches);


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

        IfThenElseLabel branches = branchStack.top();
        a.comment(" ; jump past else block");
        a.jmp(branches.elseLabel); // Jump to the code after the ELSE block
        a.comment(" ; ----- label for ELSE");
        a.bind(branches.ifLabel);
        branches.hasElse = true;
        branchStack.pop();
        branchStack.push(branches);
    }

    static void genThen()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genThen: Assembler not initialized");
        }


        auto& a = *jc.assembler;

        IfThenElseLabel branches = branchStack.top();
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
        branchStack.pop();
    }


    static void genExit()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_exit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_exit");
        a.nop();

        // First, check if there is any BEGIN... construct to exit from
        if (!beginAgainRepeatUntilStack.empty())
        {
            auto label = beginAgainRepeatUntilStack.top(); // Ensure this is not a const object

            // Depending on the construct, jump to the appropriate label
            if (label.hasAgain)
            {
                a.comment(" ; ---- again exit");
                a.jmp(label.againLabel);
            }
            else if (label.hasRepeat)
            {
                a.comment(" ; ---- repeat exit");
                a.jmp(label.repeatLabel);
            }
            else if (label.hasUntil)
            {
                // Bind a new label for `UNTIL` and jump there (needed to handle UNTIL case correctly)
                label.untilLabel = a.newLabel();
                a.comment(" ; ---- until exit");
                a.jmp(label.untilLabel);
            }
            else if (label.hasWhile)
            {
                a.comment(" ; ---- while exit");
                a.jmp(label.whileLabel);
            }
            else
            {
                throw std::runtime_error("gen_exit: No appropriate label found in beginAgainRepeatUntilStack");
            }

            // Pop the loop construct as we are exiting it
            beginAgainRepeatUntilStack.pop();
        } // If not within a loop, check for function exit
        else if (!functionsStack.empty())
        {
            a.comment(" ; ---- function exit");
            const auto& label = functionsStack.top();
            a.jmp(label.exitLabel);
        }
        else
        {
            throw std::runtime_error("gen_exit: No loop or function to exit from");
        }
    }

    // misc forth generators

    static void genSub()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genSub: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genSub");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values from the stack, subtract the first from the second, and push the result

        a.comment(" ; Pop two values from the stack");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.sub(asmjit::x86::qword_ptr(stackPtr), firstVal); // Subtract first value from second value
    }


    static void genPlus()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genPlus: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genPlus");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values from the stack, add them, and push the result

        a.comment(" ; Add two values from the stack");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer
        a.comment(" ; Add firstval to the stack");
        a.add(asmjit::x86::qword_ptr(stackPtr), firstVal); // Add first value to second value
    }

    static void genDiv()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDiv: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDiv");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp dividend = asmjit::x86::rax;
        asmjit::x86::Gp divisor = asmjit::x86::rcx;

        // Pop two values from the stack, divide the first by the second, and push the quotient

        a.comment(" ; Perform / division");
        a.mov(divisor, asmjit::x86::qword_ptr(stackPtr)); // Load the second value (divisor)
        a.add(stackPtr, 8); // Adjust stack pointer

        a.mov(dividend, asmjit::x86::qword_ptr(stackPtr)); // Load the first value (dividend)
        a.add(stackPtr, 8); // Adjust stack pointer

        a.mov(asmjit::x86::rdx, 0); // Clear RDX for unsigned division
        // RAX already contains the dividend (first value)

        a.idiv(divisor); // Perform signed division: RDX:RAX / divisor

        a.sub(stackPtr, 8); // Adjust stack pointer back
        a.mov(asmjit::x86::qword_ptr(stackPtr), asmjit::x86::rax); // Store quotient back on stack
    }

    static void genMul()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genMul: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genMul");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp secondVal = asmjit::x86::rdx;

        // Pop two values from the stack, multiply them, and push the result

        a.comment(" ; * -multiply two values from the stack");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.mov(secondVal, asmjit::x86::qword_ptr(stackPtr)); // Load second value

        a.imul(firstVal, secondVal); // Multiply first and second value, result in firstVal

        a.comment(" ; Push the result onto the stack");
        a.mov(asmjit::x86::qword_ptr(stackPtr), firstVal); // Store result back on stack
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

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; Pop two values compare them and push the result (0 or -1)");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.cmp(asmjit::x86::qword_ptr(stackPtr), firstVal); // Compare with second value
        a.sete(asmjit::x86::al); // Set AL to 1 if equal

        a.neg(asmjit::x86::al); // Set AL to -1 if true, 0 if false
        a.movsx(asmjit::x86::rax, asmjit::x86::al); // Sign-extend AL to RAX (-1 or 0)
        a.mov(asmjit::x86::qword_ptr(stackPtr), asmjit::x86::rax); // Store result on stack
    }

    static void genLt()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genLt: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genLt");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp secondVal = asmjit::x86::rdx;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; < compare them and push the result (0 or -1)");

        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.mov(secondVal, asmjit::x86::qword_ptr(stackPtr)); // Load second value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.cmp(secondVal, firstVal); // Compare second value to first value
        a.setl(asmjit::x86::al); // Set AL to 1 if secondVal < firstVal

        a.movzx(asmjit::x86::rax, asmjit::x86::al); // Zero-extend AL to RAX
        a.neg(asmjit::x86::rax); // Set RAX to -1 if true (1 -> -1), 0 remains 0

        a.sub(stackPtr, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(stackPtr), asmjit::x86::rax); // Store result on stack
    }

    static void genGt()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genGt: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genGt");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; > compare them and push the result (0 or -1)");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.cmp(asmjit::x86::qword_ptr(stackPtr), firstVal); // Compare with second value
        a.setg(asmjit::x86::al); // Set AL to 1 if greater

        a.neg(asmjit::x86::al); // Set AL to -1 if true, 0 if false
        a.movsx(asmjit::x86::rax, asmjit::x86::al); // Sign-extend AL to RAX (-1 or 0)
        a.mov(asmjit::x86::qword_ptr(stackPtr), asmjit::x86::rax); // Store result on stack
    }

    static void genNot()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genNot: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genNot");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;

        // Load the value from the stack
        a.mov(asmjit::x86::rax, asmjit::x86::qword_ptr(stackPtr)); // Load value

        // Use the `NOT` operation to flip the bits
        a.not_(asmjit::x86::rax); // Perform NOT operation

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(stackPtr), asmjit::x86::rax); // Store result back on stack
    }

    static void genAnd()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genAnd: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genAnd");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform AND, and push the result
        a.comment(" ; AND two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.and_(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Perform AND with second value

        a.mov(asmjit::x86::qword_ptr(stackPtr), firstVal); // Store result back on stack
    }


    static void genOR()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genOR: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genOR");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform OR, and push the result
        a.comment(" ; OR two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.or_(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Perform OR with second value

        a.mov(asmjit::x86::qword_ptr(stackPtr), firstVal); // Store result back on stack
    }


    static void genXOR()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genXOR: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genXOR");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp firstVal = asmjit::x86::rax;

        // Pop two values, perform XOR, and push the result
        a.comment(" ; XOR two values and push the result");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.xor_(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Perform XOR with second value

        a.mov(asmjit::x86::qword_ptr(stackPtr), firstVal); // Store result back on stack
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

        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // stack pointer in r11
        asmjit::x86::Gp tempReg = asmjit::x86::rax; // temporary register to hold the stack pointer value

        // Move the stack pointer to the temporary register
        a.comment(" ; SP@ - Get the stack pointer value");
        a.mov(tempReg, stackPtr);

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

        // Assuming r11 is the stack pointer
        a.comment(" ; drop top value");
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        a.add(stackPtr, 8); // Adjust stack pointer to drop the top value
    }


    static void genDup()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genDup: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genDup");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp topValue = asmjit::x86::rax;

        // Duplicate the top value on the stack
        a.comment(" ; Duplicate the top value on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(stackPtr)); // Load top value
        a.sub(stackPtr, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(stackPtr), topValue); // Push duplicated value
    }

    static void genSwap()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genSwap: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genSwap");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp topValue = asmjit::x86::rax;
        asmjit::x86::Gp secondValue = asmjit::x86::rcx;

        // Swap the top two values on the stack
        a.comment(" ; Swap top two values on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(stackPtr)); // Load top value
        a.mov(secondValue, asmjit::x86::qword_ptr(stackPtr, 8)); // Load second value
        a.mov(asmjit::x86::qword_ptr(stackPtr), secondValue); // Store second value in place of top value
        a.mov(asmjit::x86::qword_ptr(stackPtr, 8), topValue); // Store top value in place of second value
    }

    static void genRot()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genRot: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genRot");

        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // stack pointer in r11
        asmjit::x86::Gp topValue = asmjit::x86::rax; // top value in rax
        asmjit::x86::Gp secondValue = asmjit::x86::rcx; // second value in rcx
        asmjit::x86::Gp thirdValue = asmjit::x86::rdx; // third value in rdx

        // Rotate the top three values on the stack
        a.comment(" ; Rotate top three values on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(stackPtr)); // Load top value (TOS)
        a.mov(secondValue, asmjit::x86::qword_ptr(stackPtr, 8)); // Load second value (NOS)
        a.mov(thirdValue, asmjit::x86::qword_ptr(stackPtr, 16)); // Load third value (TOS+2)
        a.mov(asmjit::x86::qword_ptr(stackPtr), thirdValue); // Store third value in place of top value (TOS = TOS+2)
        a.mov(asmjit::x86::qword_ptr(stackPtr, 16), secondValue);
        // Store second value into third position (TOS+2 = NOS)
        a.mov(asmjit::x86::qword_ptr(stackPtr, 8), topValue); // Store top value into second position (NOS = TOS)
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

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp secondValue = asmjit::x86::rax;

        // Duplicate the second value on the stack
        a.comment(" ; Duplicate the second value on the stack");
        a.mov(secondValue, asmjit::x86::qword_ptr(stackPtr, 8)); // Load second value
        a.sub(stackPtr, 8); // Adjust stack pointer
        a.mov(asmjit::x86::qword_ptr(stackPtr), secondValue); // Push duplicated value
    }


    static void genTuck()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genTuck: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genTuck");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp topValue = asmjit::x86::rax;
        asmjit::x86::Gp secondValue = asmjit::x86::rcx;

        // Tuck the top value under the second value on the stack
        a.comment(" ; Tuck the top value under the second value on the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(stackPtr)); // Load top value
        a.mov(secondValue, asmjit::x86::qword_ptr(stackPtr, 8)); // Load second value
        a.sub(stackPtr, 8); // Adjust stack pointer to make space
        a.mov(asmjit::x86::qword_ptr(stackPtr), topValue); // Push top value
        a.mov(asmjit::x86::qword_ptr(stackPtr, 8), secondValue); // Access memory at stackPtr + 8
        a.mov(asmjit::x86::qword_ptr(stackPtr, 16), topValue); // Access memory at stackPtr + 16
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

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp topValue = asmjit::x86::rax;

        // NIP's operation is to remove the second item on the stack
        a.comment(" ; Remove the second item from the stack");
        a.mov(topValue, asmjit::x86::qword_ptr(stackPtr)); // Load top value
        a.add(stackPtr, 8); // Adjust stack pointer to skip the second item
        a.mov(asmjit::x86::qword_ptr(stackPtr, 0), topValue); // Move top value to the new top position
    }

    static void genPick(int n)
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("genPick: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- genPick");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;
        asmjit::x86::Gp value = asmjit::x86::rax;

        // Pick the nth value from the stack
        a.comment(" ; Pick the nth value from the stack");

        // Calculate the address offset for the nth element
        int offset = n * 8;

        a.mov(value, asmjit::x86::qword_ptr(stackPtr, offset)); // Load the nth value
        a.sub(stackPtr, 8); // Adjust stack pointer for pushing value
        a.mov(asmjit::x86::qword_ptr(stackPtr), value); // Push the picked value onto the stack
    }


    // Helper function to generate code for pushing constants onto the stack
    static void genPushConstant(uint64_t value)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for the constant

        // Load constant into tempValue register
        a.mov(tempValue, value);

        // Decrement stack pointer to make space
        a.sub(stackPtr, 8);

        // Push the value onto the stack
        a.mov(asmjit::x86::qword_ptr(stackPtr), tempValue);
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


    static void gen1Inc() {
        if (!jc.assembler) {
            throw std::runtime_error("gen1inc: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen1inc - use inc instruction");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;

        // Increment the value at the memory location pointed to by r11
        a.inc(asmjit::x86::qword_ptr(stackPtr));
    }


    static void gen1Dec() {
        if (!jc.assembler) {
            throw std::runtime_error("gen1inc: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen1inc - use dec instruction");

        // Assuming r11 is the stack pointer
        asmjit::x86::Gp stackPtr = asmjit::x86::r11;

        // Decrement the value at the memory location pointed to by r11
        a.dec(asmjit::x86::qword_ptr(stackPtr));
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
        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value
        asmjit::x86::Gp tempResult = asmjit::x86::rdx; // Temporary register for intermediate result

        a.comment("; multiply by ten");
        // Load the top stack value into tempValue
        a.mov(tempValue, asmjit::x86::qword_ptr(stackPtr));

        // Perform the shift left by 3 (Value * 8)
        a.mov(tempResult, tempValue);
        a.shl(tempResult, 3);

        // Perform the shift left by 1 (Value * 2)
        a.shl(tempValue, 1);

        // Add the two shifted values
        a.add(tempResult, tempValue);

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(stackPtr), tempResult);
    }


    // Helper function for left shifts
    static void genLeftShift(int shiftAmount)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value

        // Load the top stack value into tempValue
        a.mov(tempValue, asmjit::x86::qword_ptr(stackPtr));

        // Perform the shift
        a.shl(tempValue, shiftAmount);

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(stackPtr), tempValue);
    }

    // Helper function for right shifts
    static void genRightShift(int shiftAmount)
    {
        auto& a = *jc.assembler;
        asmjit::x86::Gp stackPtr = asmjit::x86::r11; // Stack pointer register
        asmjit::x86::Gp tempValue = asmjit::x86::rax; // Temporary register for value

        // Load the top stack value into tempValue
        a.mov(tempValue, asmjit::x86::qword_ptr(stackPtr));

        // Perform the shift
        a.shr(tempValue, shiftAmount);

        // Store the result back on the stack
        a.mov(asmjit::x86::qword_ptr(stackPtr), tempValue);
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

private:
    // Private constructor to prevent instantiation
    JitGenerator() = default;
    // Private destructor if needed
    ~JitGenerator() = default;
};

#endif //JITGENERATOR_H
