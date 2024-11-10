# Conditional Escape Key Checking in Loops

## Introduction

In this program, when the `*loopcheck on` option is enabled in the interactive interpreter, we compile a check of the Windows Escape key into specific parts of the loop constructs. 
This helps in providing an easy way to exit loops interactively.

## Affected Loop Constructs

The following loop constructs will incorporate Escape key checks when `*loopcheck on` is selected:

1. **Again Loops**
2. **Repeat Loops**
3. **Do Loops**

### Loop Constructs Explained

#### Begin-Again Loop

The `begin` and `again` construct forms an infinite loop that can be interrupted by an Escape key press when `*loopcheck on` is active.

#### Begin-Repeat Loop

The `begin` and `repeat` construct is typically used with a condition, and will include an Escape key check when `*loopcheck on` is active.

#### Do-Loop

The `do` and `loop` constructs define a counting loop, which will check for an Escape key press to exit early when `*loopcheck on` is active.

##### Leave

In forth the Leave word will conditionally exit a loop, it may be used inside an IF THEN statement.
The escape key check uses LEAVE to leave the loop, handling the exit gracefully.


 
## User Documentation Summary

When the `*loopcheck on` option is selected in the interactive interpreter:
- Escape key checks are compiled in the `again` and `repeat` parts of `begin` loops and the `loop` and `+loop` parts of `do` loops.

This allows users to gracefully exit loops by pressing the Escape key, aiding in debugging and interactive exploration of code.