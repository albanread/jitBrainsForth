//
// Created by oberon on 07/11/2024.
//

#ifndef TESTS_H
#define TESTS_H

void run_word(const std::string& word)
{
    ForthWord* w = d.findWord(word.c_str());
    if (w == nullptr)
    {
        if (is_number(word))
        {
            uint64_t num = std::stoll(word);
            //printf("Pushing number: %llu\n", num);
            sm.pushDS(num);
        }
        else
        {
            std::cout << "Word not found " << word << std::endl;
            return;
        }
    }
    else
    {
        w->compiledFunc();
    }
}

void run_words(const std::string& words)
{
    std::vector<std::string> words_vec;
    std::stringstream ss(words);
    std::string word;
    while (std::getline(ss, word, ' '))
    {
        if (word.empty()) continue;
        words_vec.push_back(word);
    }
    for (auto& w : words_vec)
    {
        run_word(w);
    }
}

void test_against_ds(const std::string& words, const uint64_t expected_top)
{
    sm.resetDS(); // to get a clean stack
    std::cout << "Running: " << words << std::endl;
    run_words(words);
    uint64_t result = sm.popDS();
    if (result != expected_top)
    {
        std::cout << "!! ---- Failed test: " << words << " Expected: " << expected_top << " but got: " << result <<
            std::endl;
    }
    else
        std::cout << "Passed test: " << words << " = " << expected_top << std::endl;
}


void testCompileAndRun(const std::string& wordName,
                       const std::string& wordDefinition,
                       const std::string& testString, int expectedResult)
{
    compileWord(wordName, wordDefinition);
    test_against_ds(testString, expectedResult);
    d.forgetLastWord();
}


void testInterpreter(const std::string& test_name, const std::string& testString,
                     const int expectedResult)
{
    interpreter(testString); // Interpret the test string input
    uint64_t result = sm.popDS();
    d.forgetLastWord();
    if (result != expectedResult)
    {
        std::cout << "!! ---- Failed test: "
            << test_name << " Expected: "
            << expectedResult
            << " but got: " << result <<
            std::endl;
    }
    else
        std::cout << "Passed test: " << test_name << " = " << expectedResult << std::endl;
}


void run_basic_tests()
{
    test_against_ds(" 16 ", 16);
    test_against_ds(" 16 16 + ", 32);
    test_against_ds(" 1 2 3 + + ", 6);
    test_against_ds(" 8 8* ", 64);
    test_against_ds(" 5 DUP * ", 25);
    test_against_ds(" 5 SQ ", 25);
    test_against_ds(" 1 2 3 OVER ", 2);
    test_against_ds(" 1 2 3 SWAP ", 2);
    test_against_ds(" 1 2 3 4 5 DEPTH ", 5);
    test_against_ds(" 1987 ", 1987);
    test_against_ds(" 1987 1+", 1988);
    test_against_ds(" 1987 1-", 1986);
    test_against_ds(" 1987 1 +", 1988);
    test_against_ds(" 1987 1 -", 1986);
    test_against_ds(" 3 4 +", 7);
    test_against_ds(" 10 2 -", 8);
    test_against_ds(" 6 3 *", 18);
    test_against_ds(" 8 2 /", 4);
    test_against_ds(" 5 DUP +", 10); // DUP duplicates 5, resulting in 5 5, then + adds them to get 10
    test_against_ds(" 1 2 SWAP", 1); // SWAP 1 2 results in 2 1, top of stack should be 1
    test_against_ds(" 1 2 OVER", 1);
    test_against_ds(" 3 4 SWAP 5", 5); // SWAP 3 4 results in 4 3, then push 5, top of stack is 5
    test_against_ds(" 2 3 4 + *", 14); // 3 + 4 = 7, 2 * 7 = 14
    test_against_ds(" 6 2 / 3 *", 9); // 6 / 2 = 3, 3 * 3 = 9
    test_against_ds(" 9 2 + 3 -", 8); // 9 + 2 = 11, 11 - 3 = 8
    test_against_ds(" 7 8 DUP + +", 23); // 8 DUP results in 7 8 8, 8 + 8 = 16, 7 + 16 = 23
    test_against_ds(" 6 4 3 OVER * +", 16); //
    test_against_ds(" 1 2 3 DROP + ", 3); //
    test_against_ds(" 0 0 +", 0);
    test_against_ds(" -1 1 +", 0);
    test_against_ds(" 1 0 -", 1);
    test_against_ds(" 1 1+ ", 2);
    test_against_ds(" 1 1- ", 0);
    test_against_ds(" 10 2 - 3 +", 11); // 10-2=8, 8+3=11
    test_against_ds("8 8/", 1); // 8 >> 3 = 1
    test_against_ds("64 8/", 8); // 64 >> 3 = 8
    test_against_ds("16 8/", 2); // 16 >> 3 = 2
    test_against_ds("7 8/", 0); // 7 >> 3 = 0
    test_against_ds("128 8/", 16); // 128 >> 3 = 16
    test_against_ds("8 4/", 2); // 8 >> 2 = 2
    test_against_ds("16 4/", 4); // 16 >> 2 = 4
    test_against_ds("64 4/", 16); // 64 >> 2 = 16
    test_against_ds("3 4/", 0); // 3 >> 2 = 0
    test_against_ds("128 4/", 32); // 128 >> 2 = 32
    test_against_ds("8 2/", 4); // 8 >> 1 = 4
    test_against_ds("16 2/", 8); // 16 >> 1 = 8
    test_against_ds("64 2/", 32); // 64 >> 1 = 32
    test_against_ds("1 2/", 0); // 1 >> 1 = 0
    test_against_ds("128 2/", 64); // 128 >> 1 = 64
    test_against_ds("1 8*", 8); // 1 << 3 = 8
    test_against_ds("2 8*", 16); // 2 << 3 = 16
    test_against_ds("4 8*", 32); // 4 << 3 = 32
    test_against_ds("8 8*", 64); // 8 << 3 = 64
    test_against_ds("16 8*", 128); // 16 << 3 = 128

    // Test if 3 is less than 5 (should be -1, which represents true in Forth)
    test_against_ds("3 5 <", -1);

    // Test if 5 is less than 3 (should be 0, which represents false in Forth)
    test_against_ds("5 3 <", 0);

    // Test if 5 is greater than 3 (should be -1, which represents true in Forth)
    test_against_ds("5 3 >", -1);

    // Test if 3 is greater than 5 (should be 0, which represents false in Forth)
    test_against_ds("3 5 >", 0);

    // Test if 5 is equal to 5 (should be -1, which represents true in Forth)
    test_against_ds("5 5 =", -1);

    // Test if 5 is equal to 3 (should be 0, which represents false in Forth)
    test_against_ds("5 3 =", 0);

    // Test >R followed by R> (should push 5 to RS and then pop it back to DS, so DS ends with 5)
    test_against_ds("5 >R R>", 5);

    // Test >R followed by R@ and R> (should push 5 to RS, duplicate it on DS, then pop it back to DS; result should be 5 on DS)
    test_against_ds("5 >R R@ R>", 5);

    // Test >R followed by R@ and another R@ (should push 5 to RS, then copy it to DS twice, then we only care about the first value; result should be 5 on DS)
    test_against_ds("5 >R R@ R@ + ", 10);


    // compiled word tests
    testCompileAndRun("testWord",
                      " 100 + ",
                      "1 testWord ",
                      101);

    testCompileAndRun("testWord",
                      " 0 11 1 do I + LOOP ",
                      " testWord ",
                      55);

    testCompileAndRun("testBeginAgain",
                      " 0 BEGIN DUP 10 < WHILE 1+ AGAIN  ",
                      " testBeginAgain ",
                      10); //

    testCompileAndRun("testBeginWhileRepeat",
                      " BEGIN DUP 10 < WHILE 1+ REPEAT ",
                      " 0 testBeginWhileRepeat ",
                      10); // Expected result when while condition fails

    testCompileAndRun("testBeginUntil",
                      " 0 BEGIN 1+ DUP 10 = UNTIL ",
                      " 10 testBeginUntil ",
                      10); // Expected result after the loop is terminated


    testCompileAndRun("testNestedIfElse",
                      " IF IF 1 ELSE 2 THEN ELSE 3 THEN ",
                      " -1 -1 testNestedIfElse ",
                      1); // Expected result is 1 because both conditions are true

    testCompileAndRun("testNestedIfElse",
                      " IF IF 1 ELSE 2 THEN ELSE 3 THEN ",
                      " -1 0 testNestedIfElse ",
                      3);

    testCompileAndRun("testNestedIfElse",
                      " IF IF 1 ELSE 2 THEN ELSE 3 THEN ",
                      " 0 0 testNestedIfElse ",
                      3);

    testCompileAndRun("testNestedIfElse",
                      " IF IF 1 ELSE 2 THEN ELSE 3 THEN ",
                      " 0 -1 testNestedIfElse ",
                      2);

    testCompileAndRun("testIfElse",
                      " IF 1 ELSE 2 THEN ",
                      " 0 testIfElse ",
                      2);

    testCompileAndRun("testIfElse",
                      " IF 1 ELSE 2 THEN ",
                      " -1 testIfElse ",
                      1);


    // test loop containing if then
    testCompileAndRun("testBeginUntilNestedIF",
                      " 0 BEGIN 1+ DUP 5 > IF 65 emit THEN DUP 10 = UNTIL ",
                      " 8 testBeginUntilNestedIF ",
                      10);


    // this is using "LEAVE" properly
    testCompileAndRun("testBeginUntilEarlyLeave",
                      " 0 BEGIN 1+ DUP 5 > IF LEAVE THEN DUP 10 = UNTIL ",
                      " 0 testBeginUntilEarlyLeave ",
                      6);


    // this is using "LEAVE" properly
    testCompileAndRun("testBeginAGAINLeave",
                      " 0 BEGIN 1+ DUP 5 > IF LEAVE THEN AGAIN ",
                      " 0 testBeginAGAINLeave ",
                      6);


    testCompileAndRun("testDoLoop",
                      " DO I LOOP ",
                      " 10 1 testDoLoop ",
                      9);

    testCompileAndRun("testDoPlusLoop",
                      " DO I 2 +LOOP ",
                      " 10 1 testDoPlusLoop ",
                      9);


    testCompileAndRun("testDoPlusLoop",
                      " DO I 2 +LOOP ",
                      " 10 1 testDoPlusLoop ",
                      9);


    testCompileAndRun("testThreeLevelDeepLoop",
                      " 3 0 DO  2 0  DO  1 0 DO I J K + + LOOP LOOP LOOP ",
                      " testThreeLevelDeepLoop ",
                      5);


    testCompileAndRun("testLocals",
                      " { a b } a b + ",
                      " 10 1 testLocals ",
                      11);

    testCompileAndRun("testLocals2",
                      " { a b | c } a b + to c c ",

                      " 10 6 testLocals2 ",
                      16);

    testCompileAndRun("testLocals3",
                      " { a b | c -- d } a b + to c c 2* to d ",

                      " 9 10 6 testLocals3 ",
                      32);
}
#endif //TESTS_H
