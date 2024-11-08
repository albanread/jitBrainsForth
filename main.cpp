#include <iostream>


#include "include/asmjit/asmjit.h"
#include "JitContext.h"
#include "ForthDictionary.h"
#include "StackManager.h"
#include "JitGenerator.h"

#include "interpreter.h"
#include "tests.h"


JitGenerator& gen = JitGenerator::getInstance();

// start to test some code generation


void add_words()
{
    // d.addWord("1", JitGenerator::push1, JitGenerator::build_forth(JitGenerator::push1), nullptr);
    d.addWord("2", JitGenerator::push2, JitGenerator::build_forth(JitGenerator::push2), nullptr);
    d.addWord("3", JitGenerator::push3, JitGenerator::build_forth(JitGenerator::push3), nullptr);
    d.addWord("4", JitGenerator::push4, JitGenerator::build_forth(JitGenerator::push4), nullptr);
    d.addWord("8", JitGenerator::push8, JitGenerator::build_forth(JitGenerator::push8), nullptr);
    jc.loggingON();
    d.addWord("16", JitGenerator::push16, JitGenerator::build_forth(JitGenerator::push16), nullptr);
    jc.loggingOFF();
    d.addWord("32", JitGenerator::push32, JitGenerator::build_forth(JitGenerator::push32), nullptr);
    d.addWord("64", JitGenerator::push64, JitGenerator::build_forth(JitGenerator::push64), nullptr);
    d.addWord("-1", JitGenerator::pushNeg1, JitGenerator::build_forth(JitGenerator::pushNeg1), nullptr);

    d.addWord("2*", JitGenerator::gen2mul, JitGenerator::build_forth(JitGenerator::gen2mul), nullptr);
    d.addWord("4*", JitGenerator::gen4mul, JitGenerator::build_forth(JitGenerator::gen4mul), nullptr);
    d.addWord("8*", JitGenerator::gen8mul, JitGenerator::build_forth(JitGenerator::gen8mul), nullptr);
    d.addWord("10*", JitGenerator::genMulBy10, JitGenerator::build_forth(JitGenerator::genMulBy10), nullptr);

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


    // Add the < comparison word
    d.addWord("<", JitGenerator::genLt, JitGenerator::build_forth(JitGenerator::genLt), nullptr);

    // Add the = comparison word
    d.addWord("=", JitGenerator::genEq, JitGenerator::build_forth(JitGenerator::genEq), nullptr);

    // Add the > comparison word
    d.addWord(">", JitGenerator::genGt, JitGenerator::build_forth(JitGenerator::genGt), nullptr);


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

    // add depth
    d.addWord("DEPTH", JitGenerator::genDepth2, JitGenerator::build_forth(JitGenerator::genDepth2), nullptr);
    // add nip and tuck words
    d.addWord("NIP", JitGenerator::genNip, JitGenerator::build_forth(JitGenerator::genNip), nullptr);
    d.addWord("TUCK", JitGenerator::genTuck, JitGenerator::build_forth(JitGenerator::genTuck), nullptr);

    // or, xor, and, not
    d.addWord("OR", JitGenerator::genOR, JitGenerator::build_forth(JitGenerator::genOR), nullptr);
    d.addWord("XOR", JitGenerator::genXOR, JitGenerator::build_forth(JitGenerator::genXOR), nullptr);
    d.addWord("AND", JitGenerator::genAnd, JitGenerator::build_forth(JitGenerator::genAnd), nullptr);
    d.addWord("NOT", JitGenerator::genNot, JitGenerator::build_forth(JitGenerator::genNot), nullptr);

    d.addWord("FORGET", JitGenerator::genForget, JitGenerator::build_forth(JitGenerator::genForget), nullptr);

    d.addWord(">R", JitGenerator::genToR, JitGenerator::build_forth(JitGenerator::genToR), nullptr);
    d.addWord("R>", JitGenerator::genRFrom, JitGenerator::build_forth(JitGenerator::genRFrom), nullptr);
    d.addWord("R@", JitGenerator::genRFetch, JitGenerator::build_forth(JitGenerator::genRFetch), nullptr);
    d.addWord("RP@", JitGenerator::genRPFetch, JitGenerator::build_forth(JitGenerator::genRPFetch), nullptr);

    d.addWord("SP@", JitGenerator::genSPFetch, JitGenerator::build_forth(JitGenerator::genSPFetch), nullptr);
    d.addWord("SP!", JitGenerator::genSPStore, JitGenerator::build_forth(JitGenerator::genSPStore), nullptr);
    d.addWord("RP!", JitGenerator::genRPStore, JitGenerator::build_forth(JitGenerator::genRPStore), nullptr);


    // Add immediate functions for control flow words
    d.addWord("IF", nullptr, nullptr, JitGenerator::genIf);
    d.addWord("THEN", nullptr, nullptr, JitGenerator::genThen);
    d.addWord("ELSE", nullptr, nullptr, JitGenerator::genElse);
    d.addWord("BEGIN", nullptr, nullptr, JitGenerator::genBegin);
    d.addWord("UNTIL", nullptr, nullptr, JitGenerator::genUntil);
    d.addWord("WHILE", nullptr, nullptr, JitGenerator::genWhile);
    d.addWord("REPEAT", nullptr, nullptr, JitGenerator::genRepeat);
    d.addWord("AGAIN", nullptr, nullptr, JitGenerator::genAgain);
    d.addWord("DO", nullptr, nullptr, JitGenerator::genDo);
    d.addWord("LOOP", nullptr, nullptr, JitGenerator::genLoop);
    d.addWord("+LOOP", nullptr, nullptr, JitGenerator::genPlusLoop);
    d.addWord("I", nullptr, nullptr, JitGenerator::genI);
    d.addWord("J", nullptr, nullptr, JitGenerator::genJ);
    d.addWord("K", nullptr, nullptr, JitGenerator::genK);
    d.addWord("EXIT", nullptr, nullptr, JitGenerator::genExit);
    d.addWord("LEAVE", nullptr, nullptr, JitGenerator::genLeave);
    d.addWord("{", nullptr, nullptr, JitGenerator::gen_leftBrace);
    d.addWord("to", nullptr, nullptr, JitGenerator::genTO);
    d.setState(128); // flag word as also available to interpreter
    d.addWord("value", nullptr, nullptr, JitGenerator::genImmediateValue);
    d.setState(128);// flag word as also available to interpreter
    d.addWord("variable", nullptr, nullptr, JitGenerator::genImmediateVariable);
    d.setState(128);// flag word as also available to interpreter
    d.addWord(".", JitGenerator::genDot, JitGenerator::build_forth(JitGenerator::genDot), nullptr);
    d.addWord("emit", JitGenerator::genEmit, JitGenerator::build_forth(JitGenerator::genEmit), nullptr);
    d.addWord(".s", nullptr, JitGenerator::dotS, nullptr);
    d.addWord("words", nullptr, JitGenerator::words, nullptr);
    d.addWord("see", nullptr, nullptr, JitGenerator::see);
    d.setState(128);

    compileWord("space", "32 emit");
    compileWord("spaces", "0 do space loop");
    compileWord("cr", "13 emit 10 emit");
    compileWord("sq", "dup * ");


}


int main()
{
    jc.loggingOFF();
    add_words();
    run_basic_tests();
    d.list_words();


    try
    {
        Quit();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
    return 0;
}
