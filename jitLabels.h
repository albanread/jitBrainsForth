#ifndef JITLABELS_H
#define JITLABELS_H


enum LoopType
{
    IF_THEN_ELSE,
    FUNCTION_ENTRY_EXIT,
    DO_LOOP,
    BEGIN_AGAIN_REPEAT_UNTIL,
    CASE_CONTROL
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


struct CaseLabel
{
    asmjit::Label caseLabel;
    asmjit::Label nextLabel;
    bool hasEndOf;

    void print() const
    {
        std::cout << "Case Label: " << caseLabel.id() << "\n";
        std::cout << "Next Label: " << nextLabel.id() << "\n";
    }
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

using LabelVariant = std::variant<IfThenElseLabel, FunctionEntryExitLabel, DoLoopLabel, BeginAgainRepeatUntilLabel,
                                  CaseLabel>;

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
#endif //JITLABELS_H
