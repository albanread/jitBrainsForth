## Requirements Specification for Local Variables Handling

### Overview
The goal is to implement local variables in a FORTH-like language
using a fully descending stack for local variables, referred to as
the Locals Stack (LS), managed in register `R9`. Local variables,
along with arguments and return values, will be defined syntactically
between `{` and `}`.

### Requirements

#### 1. Immediate Word: `{`
- The `{` word will be an immediate word, called by the compiler.
- It will signal the beginning of a local variable declaration block.
- Syntax format: `{ arguments | local_vars -- return_values }`
    - Arguments (`arguments`) are variables taken from the data stack.
    - Local variables (`local_vars`) are variables allocated on the LS
      (`R9`).
    - Return values (`return_values`) specify what values the word will
      return to the data stack.

#### 2. Immediate Word: `}`
- The `}` word will signal the end of the local variable declaration
  block.
- No further local variables will be allocated beyond this point
  within the current scope.

#### 3. Compile-time Local Variable Stack
- A compile-time stack will hold mappings of local variable names to
  their offsets in the LS.
- The stack will update as local variables are pushed and will reset
  on encountering the `}` word.

#### 4. Local Variables Usage
- Local variables will be accessible within the scope of the word in
  which they are defined.
- The offsets in the LS will be utilized to access these variables
  during execution.

### Detailed Behavior

1. **Starting Local Variable Declaration Block `{`**
   ```forth
   : example { a b | c d -- x y }
   ```
    - `{` is encountered, signaling the start of a local variable block.
    - `a` and `b` are identified as arguments taken from the data stack.
    - `c` and `d` are identified as local variables to be allocated on
      the LS (`R9`).
    - `x` and `y` are defined as return values to be pushed onto the
      data stack.

2. **Allocating Space for Local Variables**
    - Upon encountering `{`, the compiler will prepare to allocate space
      for local variables on the LS (`R9`).
    - Each local variable will be assigned an offset within the LS.
        - For instance, `c` at `0`, `d` at `-8` if 8 bytes are used per
          variable.

3. **Ending Local Variable Declaration Block `}`**
   ```forth
   ... }
   ```
    - The `}` word is encountered, marking the end of the local variable
      declaration block.
    - No further local variables will be added beyond this point within
      the current scope.

4. **Accessing Local Variables**
    - Within the word’s definition, local variables `c` and `d` can be
      accessed using their offsets from the LS.
    - Arguments `a` and `b` are accessed from the data stack according
      to the standard FORTH conventions.

### Example Usage

```forth
: example { a b | c d -- x y }
    a b + c !
    c @ d !
    d @ 2 * x !
    x @ 3 + y !
    c @ d @ + 10 = if
        x @ y @ +
    then
}
```

- `{` starts the local variable block.
- `a` and `b` are arguments taken from the data stack.
- `c` and `d` are local variables allocated on the LS.
- `x` and `y` are return values to be placed on the data stack.
- Local variables are manipulated within the word.
- `}` ends the local variable scope declaration.

### Assumptions and Constraints
- The implementation assumes a fully descending Locals Stack (LS)
  managed in register `R9`.
- The number and sizes of local variables must be managed within the
  available stack space.
- Local variable offsets must be calculated and managed correctly by
  the compiler.

### Implementation Notes
- Proper care must be taken to adjust the stack pointer correctly to
  avoid stack corruption.
- Compiler modifications should ensure local variables are scoped
  correctly and variable names are resolved during compile time.

By adhering to these requirements, local variables can be implemented
effectively within your FORTH-like language, leveraging the Locals
Stack (LS) for efficient storage and access.

## Implementation Notes for Local Variables Handling in FORTH

### Overview

This document describes a method for implementing local variables
within a FORTH-like language using a locals stack (LS) managed in
register `R9`. The process involves copying arguments from the data
stack (DS) to the LS, managing local variables on the LS, and copying
return values back to the data stack upon function exit.

### Implementation Steps

#### 1. Entering the Word (Function Entry)
- **Copy Arguments to Locals Stack (LS)**:
    - On entering the function, arguments from the data stack are copied
      to the locals stack.
    - This process frees up the data stack for normal operations while
      using local variables.

#### 2. Creating Local Variables
- **Allocate Local Variables on the LS**:
    - Local variables are allocated on the locals stack.
    - Each local variable is assigned a specific offset from the base of
      the LS (managed by `R9`).

#### 3. Function Execution
- **Working with Locals**:
    - During the execution of the function, the locals and arguments can
      be accessed directly from the LS.
    - Operations can be performed using these local values, just as if
      they were regular stack items.

#### 4. Exiting the Word (Function Exit)
- **Prepare Return Values**:
    - Before exiting, return values are prepared and pushed onto the LS.
- **Copy Return Values to Data Stack**:
    - The prepared return values are copied from the LS back to the data
      stack.
- **Restore Stack State**:
    - Any local variables are discarded, and the state of the data stack
      and LS is restored as required.

### Conclusion
This implementation ensures all local variables and stack arguments
are managed within the locals stack (LS) without cluttering the data
stack. It maintains a clear separation of responsibilities and
provides a robust mechanism for handling local variables and function
arguments, ensuring the integrity of both the data and locals stacks.
