#include <iostream>
#include <math.h>

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
    d.addWord("1", JitGenerator::push1, JitGenerator::build_forth(JitGenerator::push1), nullptr);
    d.addWord("2", JitGenerator::push2, JitGenerator::build_forth(JitGenerator::push2), nullptr);
    d.addWord("3", JitGenerator::push3, JitGenerator::build_forth(JitGenerator::push3), nullptr);
    d.addWord("4", JitGenerator::push4, JitGenerator::build_forth(JitGenerator::push4), nullptr);
    d.addWord("8", JitGenerator::push8, JitGenerator::build_forth(JitGenerator::push8), nullptr);
    d.addWord("16", JitGenerator::push16, JitGenerator::build_forth(JitGenerator::push16), nullptr);
    d.addWord("32", JitGenerator::push32, JitGenerator::build_forth(JitGenerator::push32), nullptr);
    d.addWord("64", JitGenerator::push64, JitGenerator::build_forth(JitGenerator::push64), nullptr);
    d.addWord("-1", JitGenerator::pushNeg1, JitGenerator::build_forth(JitGenerator::pushNeg1), nullptr);

    d.addWord("2*", JitGenerator::gen2mul, JitGenerator::build_forth(JitGenerator::gen2mul), nullptr);
    d.addWord("4*", JitGenerator::gen4mul, JitGenerator::build_forth(JitGenerator::gen4mul), nullptr);
    d.addWord("8*", JitGenerator::gen8mul, JitGenerator::build_forth(JitGenerator::gen8mul), nullptr);
    d.addWord("16*", JitGenerator::gen16mul, JitGenerator::build_forth(JitGenerator::gen16mul), nullptr);

    d.addWord("2/", JitGenerator::gen2Div, JitGenerator::build_forth(JitGenerator::gen2Div), nullptr);
    d.addWord("4/", JitGenerator::gen4Div, JitGenerator::build_forth(JitGenerator::gen4Div), nullptr);
    d.addWord("8/", JitGenerator::gen8Div, JitGenerator::build_forth(JitGenerator::gen8Div), nullptr);

    d.addWord("1+", JitGenerator::gen1Inc, JitGenerator::build_forth(JitGenerator::gen1Inc), nullptr);
    d.addWord("2+", JitGenerator::gen2Inc, JitGenerator::build_forth(JitGenerator::gen2Inc), nullptr);
    d.addWord("16+", JitGenerator::gen16Inc, JitGenerator::build_forth(JitGenerator::gen16Inc), nullptr);

    d.addWord("1-", JitGenerator::gen1Dec, JitGenerator::build_forth(JitGenerator::gen1Dec), nullptr);
    d.addWord("2-", JitGenerator::gen2Dec, JitGenerator::build_forth(JitGenerator::gen2Dec), nullptr);
    d.addWord("16-", JitGenerator::gen16Dec, JitGenerator::build_forth(JitGenerator::gen16Dec), nullptr);

    d.addWord("+", JitGenerator::genPlus, JitGenerator::build_forth(JitGenerator::genPlus), nullptr);
    d.addWord("-", JitGenerator::genSub, JitGenerator::build_forth(JitGenerator::genSub), nullptr);
    d.addWord("*", JitGenerator::genMul, JitGenerator::build_forth(JitGenerator::genMul), nullptr);
    d.addWord("/", JitGenerator::genDiv, JitGenerator::build_forth(JitGenerator::genDiv), nullptr);
    d.addWord("DUP", JitGenerator::genDup, JitGenerator::build_forth(JitGenerator::genDup), nullptr);
    d.addWord("DROP", JitGenerator::genDrop, JitGenerator::build_forth(JitGenerator::genDrop), nullptr);
    d.addWord("SWAP", JitGenerator::genSwap, JitGenerator::build_forth(JitGenerator::genSwap), nullptr);
    d.addWord("OVER", JitGenerator::genOver, JitGenerator::build_forth(JitGenerator::genOver), nullptr);
    d.addWord("ROT", JitGenerator::genRot, JitGenerator::build_forth(JitGenerator::genRot), nullptr);
    d.addWord("SP@", JitGenerator::genDSAT, JitGenerator::build_forth(JitGenerator::genDSAT), nullptr);
    // add SPBASE
    d.addWord("SPBASE", JitGenerator::SPBASE, JitGenerator::build_forth(JitGenerator::SPBASE), nullptr);

    // add nip and tuck words
    d.addWord("NIP", JitGenerator::genNip, JitGenerator::build_forth(JitGenerator::genNip), nullptr);
    d.addWord("TUCK", JitGenerator::genTuck, JitGenerator::build_forth(JitGenerator::genTuck), nullptr);

    // or, xor, and, not
    d.addWord("OR", JitGenerator::genOR, JitGenerator::build_forth(JitGenerator::genOR), nullptr);
    d.addWord("XOR", JitGenerator::genXOR, JitGenerator::build_forth(JitGenerator::genXOR), nullptr);
    d.addWord("AND", JitGenerator::genAnd, JitGenerator::build_forth(JitGenerator::genAnd), nullptr);
    d.addWord("NOT", JitGenerator::genNot, JitGenerator::build_forth(JitGenerator::genNot), nullptr);
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


void run_word(const std::string& word)
{
    std::cout << std::endl;
    std::cout << "Running word: " << word << std::endl;
    ForthWord* w = d.findWord(word.c_str());
    if (w == nullptr)
    {
        if (is_number(word)){

            uint64_t num = std::stoll(word);
            printf("Pushing number: %llu\n", num);
            sm.pushDS(num);
        }
        else
        {
            std::cout << "Word not found " << word << std::endl;
            return;
        }
        JitGenerator::exec(w->compiledFunc);
    }
}

void run_words(const std::string& words)
{
    // split the string by spaces
    std::vector<std::string> words_vec;
    std::stringstream ss(words);
    std::string word;
    while (std::getline(ss, word, ' '))
    {
        words_vec.push_back(word);
    }
    for (auto& w : words_vec)
    {
        run_word(w);
    }
}

void test_against_ds(std::string words, uint64_t expected_top)
{
    std::cout << "Running: " << words << std::endl;
    run_words(words);
    uint64_t result = sm.popDS();
    if (result != expected_top)
    {
        std::cout << "Failed test: " << words << " Expected: " << expected_top << " but got: " << result <<
            std::endl;
    }
    else
        std::cout << "Passed test: " << words << std::endl;
}


void run_basic_tests()
{
    test_against_ds("16 16 + ", 32);
    test_against_ds("1 2 3 + + ", 6);
    test_against_ds("1 2 3 ROT  ", 1);
    test_against_ds("8 8  - ", 0);
}


int main()
{
    try
    {

        std::cout << "hello" << std::endl;


        add_words();
        d.list_words();

        run_basic_tests();
        sm.display_stack();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;

        sm.display_stack();
    }
    return 0;
}
