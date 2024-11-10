# Stack Helper Functions Documentation

## Introduction

This document provides an overview of the helper functions for manipulating various stack types in a Forth language variant using ASMJIT. These functions help manage values in different stack registers, specifically `R15` for the Data Stack (DS), `R14` for the Return Stack (RS), and `R12` for the String Stack (SS).

## Function List

1. **Data Stack (DS) Functions**
    - `pushDS(asmjit::x86::Gp reg)`
    - `popDS(asmjit::x86::Gp reg)`
    - `loadDS(void* dataAddress)`
    - `loadFromDS()`
    - `storeDS(void* dataAddress)`
    - `storeFromDS()`

2. **Return Stack (RS) Functions**
    - `pushRS(asmjit::x86::Gp reg)`
    - `popRS(asmjit::x86::Gp reg)`

3. **String Stack (SS) Functions**
    - `pushSS(asmjit::x86::Gp reg)`
    - `popSS(asmjit::x86::Gp reg)`
    - `loadSS(void* dataAddress)`
    - `loadFromSS()`
    - `storeSS(void* dataAddress)`
    - `storeFromSS()`

## Description of Each Function

### Data Stack (DS) Functions

#### `pushDS(asmjit::x86::Gp reg)`
**Description:** Pushes the value in the given register onto the Data Stack (R15).

**Usage Example:**
```c++
pushDS(asmjit::x86::eax);
```

#### `popDS(asmjit::x86::Gp reg)`
**Description:** Pops a value from the Data Stack (R15) and stores it in the given register.

**Usage Example:**
```c++
popDS(asmjit::x86::eax);
```

#### `loadDS(void* dataAddress)`
**Description:** Loads a value from the specified memory address and pushes it onto the Data Stack (R15).

**Usage Example:**
```c++
loadDS(&someVariable);
```

#### `loadFromDS()`
**Description:** Pops an address from the Data Stack (R15), dereferences it to get the value, and then pushes the value back onto the Data Stack.

**Usage Example:**
```c++
loadFromDS();
```

#### `storeDS(void* dataAddress)`
**Description:** Pops a value from the Data Stack (R15) and stores it at the specified memory address.

**Usage Example:**
```c++
storeDS(&someVariable);
```

#### `storeFromDS()`
**Description:** Pops an address from the Data Stack (R15) and then pops a value from the Data Stack. Stores the value at the specified address.

**Usage Example:**
```c++
storeFromDS();
```

### Return Stack (RS) Functions

#### `pushRS(asmjit::x86::Gp reg)`
**Description:** Pushes the value in the given register onto the Return Stack (R14).

**Usage Example:**
```c++
pushRS(asmjit::x86::eax);
```

#### `popRS(asmjit::x86::Gp reg)`
**Description:** Pops a value from the Return Stack (R14) and stores it in the given register.

**Usage Example:**
```c++
popRS(asmjit::x86::eax);
```

### String Stack (SS) Functions

#### `pushSS(asmjit::x86::Gp reg)`
**Description:** Pushes the value in the specified register onto the String Stack (R12).

**Usage Example:**
```c++
pushSS(asmjit::x86::eax);
```

#### `popSS(asmjit::x86::Gp reg)`
**Description:** Pops a value from the String Stack (R12) and stores it in the given register.

**Usage Example:**
```c++
popSS(asmjit::x86::eax);
```

#### `loadSS(void* dataAddress)`
**Description:** Loads a value from the specified address and pushes it onto the String Stack (R12).

**Usage Example:**
```c++
loadSS(&someVariable);
```

#### `loadFromSS()`
**Description:** Pops an address from the String Stack (R12), dereferences it to get the value, and then pushes the value back onto the String Stack.

**Usage Example:**
```c++
loadFromSS();
```

#### `storeSS(void* dataAddress)`
**Description:** Pops a value from the String Stack (R12) and stores it at the specified memory address.

**Usage Example:**
```c++
storeSS(&someVariable);
```

#### `storeFromSS()`
**Description:** Pops an address from the String Stack (R12) and then pops a value from the String Stack. Stores the value at the specified address.

**Usage Example:**
```c++
storeFromSS();
```

## Summary

These stack helper functions facilitate managing values in different stacks by providing push, pop, load, and store operations. They help abstract the low-level assembly operations required to manipulate stack values directly using specific registers (`R15`, `R14`, `R12`).