#ifndef JITGENERATOR_H
#define JITGENERATOR_H

#include <functional>
#include <iostream>
#include <stdexcept>
#include "include/asmjit/asmjit.h"
#include "JitContext.h"
#include "ForthDictionary.h"
#include <stack>

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


inline JitContext& jc = JitContext::getInstance();
inline ForthDictionary& d = ForthDictionary::getInstance();
inline StackManager& sm = StackManager::getInstance();

inline extern void prim_emit(const uint64_t a)
{
    printf("; prim_emit: %d\n", a);
    putchar(a);
}

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
        std::cout << "; gen_prologue\n";
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_prologue: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_prologue");
        a.nop();
        entryFunction();
        a.comment(" ; ----- FUNCTION_ENTRY label");
        FunctionEntryExitLabel label;
        label.entryLabel = a.newLabel();
        a.bind(label.entryLabel);
        label.exitLabel = a.newLabel();
        // save on functionsStack
        functionsStack.push(label);


        printf(" ; gen_prologue: %p\n", jc.assembler);
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

        a.comment(" ; ----- FUNCTION_EXIT label");

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
        std::cout << "; building forth function ... \n";
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

    static bool gen_do()
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
        return true;
    }

    static bool gen_loop()
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
        a.jle(loopLabel.doLabel);
        // jump to loop start if current index is less than or equal to limit

        a.comment(" ; ----- LEAVE and loop label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        // drop the current index and limit from the return stack
        a.comment(" ; ----- drop loop counters");
        popRS(currentIndex);
        popRS(limit);
        a.nop(); // no-op

        return true;
    }


    static bool gen_plus_loop()
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
        a.jle(loopLabel.doLabel);

        a.comment(" ; ----- Loop and Leave label");
        a.bind(loopLabel.loopLabel);
        a.bind(loopLabel.leaveLabel);

        a.nop(); // no-op

        // Clean up loop stack

        return true;
    }

    static bool gen_leave()
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

        return true;
    }

    static bool gen_I()
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

        return true;
    }

    static bool gen_J()
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

        return true;
    }

    static bool gen_K()
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

        return true;
    }

    /*


    static bool gen_recurse()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_recurse: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_recurse");
        a.nop();

        // Retrieve the FUNCTION_ENTRY label of the current word

        if (!functionEntryLabel.isValid())
        {
            throw std::runtime_error("gen_recurse: FUNCTION_ENTRY label is invalid or undefined");
        }

        // call FUNCTION_ENTRY label to recurse
        a.call(functionEntryLabel);

        return true;
    }


    static bool gen_tail_recurse()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_recurse: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_recurse");
        a.nop();

        // Retrieve the FUNCTION_ENTRY label of the current word

        if (!functionEntryLabel.isValid())
        {
            throw std::runtime_error("gen_tail_recurse: FUNCTION_ENTRY label is invalid or undefined");
        }

        // Jump to the FUNCTION_ENTRY label to recurse
        a.jmp(functionEntryLabel);

        return true;
    }
    */


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

    static bool genElse()
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
        return true;
    }

    static bool genThen()
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
        return true;
    }

    static bool gen_exit()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_exit: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_exit");
        a.nop();

        // check if functionsStack is empty
        if (functionsStack.empty())
        {
            throw std::runtime_error("gen_exit: functionsStack is empty");
        }

        const auto& label = functionsStack.top();
        a.jmp(label.exitLabel);

        return true;
    }


    /*
    static bool gen_begin()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_begin: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_begin");
        a.nop();

        // Create a label for the BEGIN loop start


        return true;
    }

    static bool gen_again()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_again: Assembler not initialized");
        }



        if (!beginLabel.isValid())
        {
            throw std::runtime_error("gen_again: Begin label is invalid or undefined");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_again");
        a.nop();

        // Jump back to the BEGIN label
        a.jmp(beginLabel);




        return true;
    }


    static bool gen_repeat()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_repeat: Assembler not initialized");
        }

        asmjit::Label beginLabel;

        // Try to get the BEGIN label
        try
        {

        }
        catch (const std::runtime_error&)
        {
            throw std::runtime_error("gen_repeat: Failed to find BEGIN label");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_repeat");
        a.nop();

        // Jump back to the BEGIN label
        a.jmp(beginLabel);

        // Pop the BEGIN label since we're done with it


        a.comment(" ; ----- REPEAT label");


        return true;
    }


    static bool gen_until()
    {


        if (!jc.assembler)
        {
            throw std::runtime_error("gen_until: Assembler not initialized");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_until");
        a.nop();

        // Use a conditional jump back to the BEGIN label
        // This will be an example of a conditional jump on a boolean value on top of the stack
        asmjit::x86::Gp flag = asmjit::x86::rax;
        popDS(flag);
        a.test(flag, flag);
        a.jz(nullptr);







        return true;
    }


    static bool gen_while()
    {
        if (!jc.assembler)
        {
            throw std::runtime_error("gen_while: Assembler not initialized");
        }

        asmjit::Label repeatLabel;

        // Try to get the REPEAT label
        try
        {
            repeatLabel = nullptr;
        }
        catch (const std::runtime_error&)
        {
            throw std::runtime_error("gen_while: Failed to find REPEAT label");
        }

        auto& a = *jc.assembler;
        a.comment(" ; ----- gen_while");
        a.nop();

        // Pop the condition flag from the data stack
        asmjit::x86::Gp flag = asmjit::x86::rax;
        popDS(flag);

        // Check the condition and jump to REPEAT label if false
        a.test(flag, flag);
        a.jz(repeatLabel);

        // Optionally push WHILE label for documentation or debugging
        a.comment(" ; ----- WHILE condition start");


        return true;
    }
*/
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
        asmjit::x86::Gp firstVal = asmjit::x86::rax;
        asmjit::x86::Gp divisor = asmjit::x86::rcx;

        // Pop two values from the stack, divide the second by the first, and push the quotient

        a.comment(" ; Perform / division");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.mov(divisor, asmjit::x86::qword_ptr(stackPtr)); // Load second value (divisor)

        a.mov(asmjit::x86::rdx, 0); // Clear RDX for division
        a.mov(asmjit::x86::rax, divisor);

        a.idiv(firstVal); // Divide second value (in RAX) by first value, quotient in RAX

        a.comment(" ; Push the quotient onto the stack");
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

        // Pop two values, compare them and push the result (0 or -1)
        a.comment(" ; < compare them and push the result (0 or -1)");
        a.mov(firstVal, asmjit::x86::qword_ptr(stackPtr)); // Load first value
        a.add(stackPtr, 8); // Adjust stack pointer

        a.cmp(asmjit::x86::qword_ptr(stackPtr), firstVal); // Compare with second value
        a.setl(asmjit::x86::al); // Set AL to 1 if less

        a.neg(asmjit::x86::al); // Set AL to -1 if true, 0 if false
        a.movsx(asmjit::x86::rax, asmjit::x86::al); // Sign-extend AL to RAX (-1 or 0)
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

private:
    // Private constructor to prevent instantiation
    JitGenerator() = default;
    // Private destructor if needed
    ~JitGenerator() = default;
};

#endif //JITGENERATOR_H
