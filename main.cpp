#include <iostream>
#include "include/asmjit/asmjit.h"
#include "JitContext.h"
#include "ForthDictionary.h"
#include "StackManager.h"
#include "JitGenerator.h"

#include "interpreter.h"


JitGenerator& gen = JitGenerator::getInstance();

// start to test some code generation


void test(int c)
{
    JitGenerator::genPrologue();
    jc.uint64_A = c;
    JitGenerator::genPushLong();
    JitGenerator::genEmit();
    JitGenerator::genEpilogue();
    const ForthFunction func = JitGenerator::end();
    func();
}


void push(int c)
{
    JitGenerator::genPrologue();
    jc.uint64_A = c;
    JitGenerator::genPushLong();
    JitGenerator::genEpilogue();
    const ForthFunction func = JitGenerator::end();
    func();
}

void test_if_else_then()
{
    JitGenerator::genPrologue();
    jc.uint64_A = 0;
    JitGenerator::genPushLong();
    JitGenerator::genIf();
    jc.uint64_A = 35;
    JitGenerator::genPushLong();
    JitGenerator::genEmit();
    JitGenerator::genElse();
    jc.uint64_A = 43;
    JitGenerator::genPushLong();
    JitGenerator::genEmit();
    JitGenerator::genThen();
    JitGenerator::genEpilogue();
    const ForthFunction func = JitGenerator::end();
    func();
}


void add_words()
{

    d.addWord("IF",
              nullptr,
              JitGenerator::genIf);

    d.addWord("emit",
              JitGenerator::build_forth(JitGenerator::genEmit),
              nullptr);


}


void test_do_loop()
{
    JitGenerator::genPrologue();
    jc.uint64_A = 10;
    JitGenerator::genPushLong();
    jc.uint64_A = 1;
    JitGenerator::genPushLong();
    JitGenerator::gen_do();

    jc.uint64_A = 35;
    JitGenerator::genPushLong();
    JitGenerator::genEmit();
    JitGenerator::gen_loop();

    JitGenerator::genEpilogue();

    const ForthFunction func = JitGenerator::end();
    func();
}

void test_do_loop_with_index()
{
    JitGenerator::genPrologue();
    jc.uint64_A = 10;
    JitGenerator::genPushLong();
    jc.uint64_A = 1;
    JitGenerator::genPushLong();
    JitGenerator::gen_do();

    jc.uint64_A = 35;
    JitGenerator::genPushLong();
    JitGenerator::gen_I();
    JitGenerator::genPlus();
    JitGenerator::genEmit();
    JitGenerator::gen_loop();

    JitGenerator::genEpilogue();

    const ForthFunction func = JitGenerator::end();
    func();
}

/*
void test_begin_if_leave_then_again_loop()
{
    JitGenerator::genPrologue();

    // Initialize counter to 12
    jc.uint64_A = 12;
    JitGenerator::genPushLong();

    // BEGIN label
    JitGenerator::gen_begin();

    // Push character '*' (ASCII 42) and emit it
    jc.uint64_A = 42;          // ASCII value for '*'
    JitGenerator::genPushLong();
    JitGenerator::genEmit();

    // Decrement counter
    jc.uint64_A = 1;
    JitGenerator::genPushLong();
    JitGenerator::genSub();

    // Duplicate counter to check for zero
    JitGenerator::genDup();

    // Check if counter is zero for IF LEAVE THEN
    jc.uint64_A = 0;           // Zero value for comparison
    JitGenerator::genPushLong();
    JitGenerator::genEq();
    JitGenerator::genIf();
    JitGenerator::gen_leave();
    JitGenerator::genThen();

    // Pop the duplicated counter to proceed with the next iteration
    JitGenerator::genDrop();

    // AGAIN (jump back to BEGIN)
    JitGenerator::gen_again();

    JitGenerator::genEpilogue();

    // Finalize and invoke the generated function
    const ForthFunction func = JitGenerator::end();
    func();
}
*/


int main()
{
    try
    {
        std::cout << "hello" << std::endl;
        //add_words();

        //test_do_loop();
        //test_do_loop_with_index();


        //sm.display_stack();

    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;

        sm.display_stack();
    }
    return 0;
}
