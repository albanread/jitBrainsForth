#include <iostream>

#include "JitContext.h"
#include "ForthDictionary.h"
#include "JitGenerator.h"
#include "interpreter.h"
#include "tests.h"
#include "quit.h"


JitGenerator& gen = JitGenerator::getInstance();

StringInterner& interner = StringInterner::getInstance();


// start to test some code generation


void add_words()
{


    d.addWord("1", JitGenerator::push1, JitGenerator::build_forth(JitGenerator::push1), nullptr,nullptr);
    d.addWord("2", JitGenerator::push2, JitGenerator::build_forth(JitGenerator::push2), nullptr, nullptr);
    d.addWord("3", JitGenerator::push3, JitGenerator::build_forth(JitGenerator::push3), nullptr, nullptr);
    d.addWord("4", JitGenerator::push4, JitGenerator::build_forth(JitGenerator::push4), nullptr, nullptr);
    d.addWord("8", JitGenerator::push8, JitGenerator::build_forth(JitGenerator::push8), nullptr, nullptr);

   // d.addWord("16", JitGenerator::push16, JitGenerator::build_forth(JitGenerator::push16), nullptr, nullptr);

    d.addWord("32", JitGenerator::push32, JitGenerator::build_forth(JitGenerator::push32), nullptr, nullptr);
    d.addWord("64", JitGenerator::push64, JitGenerator::build_forth(JitGenerator::push64), nullptr, nullptr);
    d.addWord("-1", JitGenerator::pushNeg1, JitGenerator::build_forth(JitGenerator::pushNeg1), nullptr, nullptr);

    d.addWord("2*", JitGenerator::gen2mul, JitGenerator::build_forth(JitGenerator::gen2mul), nullptr, nullptr);
    d.addWord("4*", JitGenerator::gen4mul, JitGenerator::build_forth(JitGenerator::gen4mul), nullptr, nullptr);
    d.addWord("8*", JitGenerator::gen8mul, JitGenerator::build_forth(JitGenerator::gen8mul), nullptr, nullptr);
    d.addWord("10*", JitGenerator::genMulBy10, JitGenerator::build_forth(JitGenerator::genMulBy10), nullptr, nullptr);

    d.addWord("16*", JitGenerator::gen16mul, JitGenerator::build_forth(JitGenerator::gen16mul), nullptr, nullptr);

    d.addWord("2/", JitGenerator::gen2Div, JitGenerator::build_forth(JitGenerator::gen2Div), nullptr, nullptr);
    d.addWord("4/", JitGenerator::gen4Div, JitGenerator::build_forth(JitGenerator::gen4Div), nullptr, nullptr);
    d.addWord("8/", JitGenerator::gen8Div, JitGenerator::build_forth(JitGenerator::gen8Div), nullptr, nullptr);

    d.addWord("1+", JitGenerator::gen1Inc, JitGenerator::build_forth(JitGenerator::gen1Inc), nullptr, nullptr);
    d.addWord("2+", JitGenerator::gen2Inc, JitGenerator::build_forth(JitGenerator::gen2Inc), nullptr, nullptr);
    d.addWord("16+", JitGenerator::gen16Inc, JitGenerator::build_forth(JitGenerator::gen16Inc), nullptr, nullptr);

    d.addWord("1-", JitGenerator::gen1Dec, JitGenerator::build_forth(JitGenerator::gen1Dec), nullptr, nullptr);
    d.addWord("2-", JitGenerator::gen2Dec, JitGenerator::build_forth(JitGenerator::gen2Dec), nullptr, nullptr);
    d.addWord("16-", JitGenerator::gen16Dec, JitGenerator::build_forth(JitGenerator::gen16Dec), nullptr, nullptr);


    // Add the < comparison word
    d.addWord("<", JitGenerator::genLt, JitGenerator::build_forth(JitGenerator::genLt), nullptr, nullptr);

    // Add the = comparison word
    d.addWord("=", JitGenerator::genEq, JitGenerator::build_forth(JitGenerator::genEq), nullptr, nullptr);

    // Add the > comparison word
    d.addWord(">", JitGenerator::genGt, JitGenerator::build_forth(JitGenerator::genGt), nullptr, nullptr);


    d.addWord("+", JitGenerator::genPlus, JitGenerator::build_forth(JitGenerator::genPlus), nullptr, nullptr);
    d.addWord("-", JitGenerator::genSub, JitGenerator::build_forth(JitGenerator::genSub), nullptr, nullptr);
    d.addWord("*", JitGenerator::genMul, JitGenerator::build_forth(JitGenerator::genMul), nullptr, nullptr);
    d.addWord("/", JitGenerator::genDiv, JitGenerator::build_forth(JitGenerator::genDiv), nullptr, nullptr);
    d.addWord("DUP", JitGenerator::genDup, JitGenerator::build_forth(JitGenerator::genDup), nullptr, nullptr);
    d.addWord("DROP", JitGenerator::genDrop, JitGenerator::build_forth(JitGenerator::genDrop), nullptr, nullptr);
    d.addWord("SWAP", JitGenerator::genSwap, JitGenerator::build_forth(JitGenerator::genSwap), nullptr, nullptr);
    d.addWord("OVER", JitGenerator::genOver, JitGenerator::build_forth(JitGenerator::genOver), nullptr, nullptr);
    d.addWord("ROT", JitGenerator::genRot, JitGenerator::build_forth(JitGenerator::genRot), nullptr, nullptr);

    // add depth
    // add nip and tuck words
    d.addWord("NIP", JitGenerator::genNip, JitGenerator::build_forth(JitGenerator::genNip), nullptr, nullptr);
    d.addWord("TUCK", JitGenerator::genTuck, JitGenerator::build_forth(JitGenerator::genTuck), nullptr, nullptr);

    // or, xor, and, not
    d.addWord("OR", JitGenerator::genOR, JitGenerator::build_forth(JitGenerator::genOR), nullptr, nullptr);
    d.addWord("XOR", JitGenerator::genXOR, JitGenerator::build_forth(JitGenerator::genXOR), nullptr, nullptr);
    d.addWord("AND", JitGenerator::genAnd, JitGenerator::build_forth(JitGenerator::genAnd), nullptr, nullptr);
    d.addWord("NOT", JitGenerator::genNot, JitGenerator::build_forth(JitGenerator::genNot), nullptr, nullptr);


    d.addWord(">R", JitGenerator::genToR, JitGenerator::build_forth(JitGenerator::genToR), nullptr, nullptr);
    d.addWord("R>", JitGenerator::genRFrom, JitGenerator::build_forth(JitGenerator::genRFrom), nullptr, nullptr);
    d.addWord("R@", JitGenerator::genRFetch, JitGenerator::build_forth(JitGenerator::genRFetch), nullptr, nullptr);
    d.addWord("RP@", JitGenerator::genRPFetch, JitGenerator::build_forth(JitGenerator::genRPFetch), nullptr, nullptr);
    d.addWord("SP", JitGenerator::genDSAT, JitGenerator::build_forth(JitGenerator::genDSAT), nullptr, nullptr);

    d.addWord("SP@", JitGenerator::genSPFetch, JitGenerator::build_forth(JitGenerator::genSPFetch), nullptr, nullptr);
    d.addWord("SP!", JitGenerator::genSPStore, JitGenerator::build_forth(JitGenerator::genSPStore), nullptr, nullptr);
    d.addWord("RP!", JitGenerator::genRPStore, JitGenerator::build_forth(JitGenerator::genRPStore), nullptr, nullptr);
    d.addWord("@", JitGenerator::genAT, JitGenerator::build_forth(JitGenerator::genAT), nullptr, nullptr);
    d.addWord("!", JitGenerator::genStore, JitGenerator::build_forth(JitGenerator::genStore), nullptr, nullptr);


    // Add immediate functions for control flow words
    d.addWord("IF", nullptr, nullptr, JitGenerator::genIf, nullptr);
    d.addWord("THEN", nullptr, nullptr, JitGenerator::genThen, nullptr);
    d.addWord("ELSE", nullptr, nullptr, JitGenerator::genElse, nullptr);
    d.addWord("BEGIN", nullptr, nullptr, JitGenerator::genBegin, nullptr);
    d.addWord("UNTIL", nullptr, nullptr, JitGenerator::genUntil, nullptr);
    d.addWord("WHILE", nullptr, nullptr, JitGenerator::genWhile, nullptr);
    d.addWord("REPEAT", nullptr, nullptr, JitGenerator::genRepeat, nullptr);
    d.addWord("AGAIN", nullptr, nullptr, JitGenerator::genAgain, nullptr);
    d.addWord("DO", nullptr, nullptr, JitGenerator::genDo, nullptr);
    d.addWord("LOOP", nullptr, nullptr, JitGenerator::genLoop, nullptr);
    d.addWord("+LOOP", nullptr, nullptr, JitGenerator::genPlusLoop, nullptr);
    d.addWord("I", nullptr, nullptr, JitGenerator::genI, nullptr);
    d.addWord("J", nullptr, nullptr, JitGenerator::genJ, nullptr);
    d.addWord("K", nullptr, nullptr, JitGenerator::genK, nullptr);
    d.addWord("EXIT", nullptr, nullptr, JitGenerator::genExit, nullptr);
    d.addWord("LEAVE", nullptr, nullptr, JitGenerator::genLeave, nullptr);
    d.addWord("{", nullptr, nullptr, JitGenerator::gen_leftBrace, nullptr);
    d.addWord("to", nullptr, nullptr, JitGenerator::genTO, JitGenerator::execTO);


    d.addWord("value", nullptr, nullptr, nullptr, JitGenerator::genImmediateValue);
    d.addWord("variable", nullptr, nullptr, nullptr, JitGenerator::genImmediateVariable);

    d.addWord("DEPTH", JitGenerator::genDepth2, JitGenerator::build_forth(JitGenerator::genDepth2), nullptr, nullptr);
    d.addWord("FORGET", JitGenerator::genForget, JitGenerator::build_forth(JitGenerator::genForget), nullptr, nullptr);
    d.addWord(".", JitGenerator::genDot, JitGenerator::build_forth(JitGenerator::genDot), nullptr, nullptr);
    d.addWord("emit", JitGenerator::genEmit, JitGenerator::build_forth(JitGenerator::genEmit), nullptr, nullptr);
    d.addWord(".s", nullptr, JitGenerator::dotS, nullptr, nullptr);
    d.addWord("words", nullptr, JitGenerator::words, nullptr, nullptr);
    d.addWord("see", nullptr, nullptr, nullptr, JitGenerator::see);

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
    Quit();
}
