# Compiler Utility Documentation

## Overview

The `CompilerUtility` file contains several utility functions 
designed to assist with the compilation and interpretation of words
within this Forth-like programming environment. 
This includes functionality for tracing execution, parsing literals 
and numbers, and handling various modes of compilation.

### Functions

---

### compileWord

```cpp
void compileWord(const std::string& wordName, const std::string& compileText, const std::string& sourceCode);
```

- **Description**: Compiles a given word from the source code.
- **Parameters**:
    - `wordName`: The name of the word to compile.
    - `compileText`: The text to compile.
    - `sourceCode`: The source code containing the word.

The Forth code will be compiled into a native code function, 
and created in the dictionary.
Where words have generators, code will be generated, words with no
generators will be compiled into function calls.



---

### traceOn

```cpp
inline void traceOn(const std::string& word);
```

- **Description**: Enables tracing for a specified word.
- **Parameters**:
    - `word`: The word for which tracing is to be enabled.

Traces the compilation process, showing the generated code.
Does not trace at runtime.
---

### traceOff

```cpp
inline void traceOff(const std::string& word);
```

- **Description**: Disables tracing for a specified word.
- **Parameters**:
    - `word`: The word for which tracing is to be disabled.

---

### clearR15

```cpp
inline void clearR15();
```

- **Description**: Clears the R15 register (context or internal state related). Specific to the implementation details of this Forth-like system.

---

### exec

```cpp
inline void exec(ForthFunction f);
```

- **Description**: Executes a given Forth function.
- **Parameters**:
    - `f`: The Forth function to execute.

---

### parseFloat

```cpp
inline double parseFloat(const std::string& word);
```

- **Description**: Parses a floating-point number from a given string.
- **Parameters**:
    - `word`: The string to parse.
- **Returns**: The parsed floating-point number.

---

### parseNumber

```cpp
inline int64_t parseNumber(const std::string& word);
```

- **Description**: Parses an integer (64-bit) number from a given string.
- **Parameters**:
    - `word`: The string to parse.
- **Returns**: The parsed integer number.

---

### scanForLiterals

```cpp
inline std::string scanForLiterals(const std::string& compileText);
```

- **Description**: Scans the compile text for literals and processes them.
- **Parameters**:
    - `compileText`: The text to scan.
- **Returns**: A processed string with literals handled.

Literal strings are interned in the strings pool.

References to the id of the string in the string pool are created.

Immediate compiling words use these references at compile time.



---

### splitAndLogWords

```cpp
inline std::vector<std::string> splitAndLogWords(const std::string& sourceCode);
```

- **Description**: Splits the source code into words and logs them.
- **Parameters**:
    - `sourceCode`: The source code to split.
- **Returns**: A vector of strings, each representing a word from the source code.

---

### handleCompileMode

```cpp
inline void handleCompileMode(size_t& i, const std::vector<std::string>& words, const std::string& sourceCode);
```

- **Description**: Handles the compilation mode, processing words from the source code.
- **Parameters**:
    - `i`: The current index in the source code.
    - `words`: A vector of words from the source code.
    - `sourceCode`: The source code.

---

### interpreterProcessWord

```cpp
inline void interpreterProcessWord(const std::string& word, size_t& i, const std::vector<std::string>& words);
```

- **Description**: Processes a word in interpreter mode.
- **Parameters**:
    - `word`: The word to process.
    - `i`: The current index in the source code.
    - `words`: A vector of words from the source code.

---

### Summary

The `CompilerUtility` file contains essential utility functions used in the compilation and interpretation processes within a Forth-like environment. These functions handle various tasks such as enabling/disabling tracing, parsing literals, managing the compilation state, and executing compiled functions.

---# Compiler Utility Documentation

## Overview

The `CompilerUtility` file contains several utility functions designed to assist with the compilation and interpretation of words within a Forth-like programming environment. This includes functionality for tracing execution, parsing literals and numbers, and handling various modes of compilation.

### Functions

---

### compileWord

```cpp
void compileWord(const std::string& wordName, const std::string& compileText, const std::string& sourceCode);
```

- **Description**: Compiles a given word from the source code.
- **Parameters**:
    - `wordName`: The name of the word to compile.
    - `compileText`: The text to compile.
    - `sourceCode`: The source code containing the word.

---

### traceOn

```cpp
inline void traceOn(const std::string& word);
```

- **Description**: Enables tracing for a specified word.
- **Parameters**:
    - `word`: The word for which tracing is to be enabled.

---

### traceOff

```cpp
inline void traceOff(const std::string& word);
```

- **Description**: Disables tracing for a specified word.
- **Parameters**:
    - `word`: The word for which tracing is to be disabled.

---

### clearR15

```cpp
inline void clearR15();
```

- **Description**: Clears the R15 register (context or internal state related). Specific to the implementation details of this Forth-like system.

---

### exec

```cpp
inline void exec(ForthFunction f);
```

- **Description**: Executes a given Forth function.
- **Parameters**:
    - `f`: The Forth function to execute.

---

### parseFloat

```cpp
inline double parseFloat(const std::string& word);
```

- **Description**: Parses a floating-point number from a given string.
- **Parameters**:
    - `word`: The string to parse.
- **Returns**: The parsed floating-point number.

---

### parseNumber

```cpp
inline int64_t parseNumber(const std::string& word);
```

- **Description**: Parses an integer (64-bit) number from a given string.
- **Parameters**:
    - `word`: The string to parse.
- **Returns**: The parsed integer number.

---

### scanForLiterals

```cpp
inline std::string scanForLiterals(const std::string& compileText);
```

- **Description**: Scans the compile text for literals and processes them.
- **Parameters**:
    - `compileText`: The text to scan.
- **Returns**: A processed string with literals handled.

---

### splitAndLogWords

```cpp
inline std::vector<std::string> splitAndLogWords(const std::string& sourceCode);
```

- **Description**: Splits the source code into words and logs them.
- **Parameters**:
    - `sourceCode`: The source code to split.
- **Returns**: A vector of strings, each representing a word from the source code.

---

### handleCompileMode

```cpp
inline void handleCompileMode(size_t& i, const std::vector<std::string>& words, const std::string& sourceCode);
```

- **Description**: Handles the compilation mode, processing words from the source code.
- **Parameters**:
    - `i`: The current index in the source code.
    - `words`: A vector of words from the source code.
    - `sourceCode`: The source code.

---

### interpreterProcessWord

```cpp
inline void interpreterProcessWord(const std::string& word, size_t& i, const std::vector<std::string>& words);
```

- **Description**: Processes a word in interpreter mode.
- **Parameters**:
    - `word`: The word to process.
    - `i`: The current index in the source code.
    - `words`: A vector of words from the source code.

---

### Summary

The `CompilerUtility` file contains essential utility functions used
in the compilation and interpretation processes within this 
Forth-like environment. 
These functions handle various tasks such as enabling/disabling tracing,
parsing literals, managing the compilation state, 
and executing compiled functions.

---