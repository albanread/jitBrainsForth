# Windows x64 Register Usage with MINGW

## x64 Calling Conventions

The x64 calling convention for Windows (Microsoft x64 calling convention) 
is distinct from the System V AMD64 ABI used in Unix-like systems. 
Here's an overview:

### Argument Passing

- The first four integer or pointer arguments are passed in the following registers:
    - `RCX`
    - `RDX`
    - `R8`
    - `R9`

- Floating-point arguments are passed in:
    - `XMM0`
    - `XMM1`
    - `XMM2`
    - `XMM3`

- Additional arguments are passed on the stack.

### Return Values

- Integers, pointers, and structures up to 8 bytes are returned in `RAX`.
- Floating-point values are returned in `XMM0`.
- Larger structs are either returned via a hidden pointer passed as the first argument or on the stack.

### Shadow Space

- The caller must allocate a 32-byte "shadow space" on the stack for the called function to use. This space is reserved regardless of the number of parameters.

### Stack Alignment

- The stack must be aligned to 16 bytes at the point of any `call` instruction. The 32-byte shadow space helps maintain this alignment.

### Preserved Registers

Certain registers are preserved across function calls, meaning the called function must save and restore their values if it uses them:
- `RBX`
- `RBP`
- `RDI`
- `RSI`
- `R12`-`R15`

### Volatile Registers

The rest of the registers are considered volatile and do not need to be preserved across function calls:
- `RAX`
- `RCX`
- `RDX`
- `R8`
- `R9`
- `R10`
- `R11`
- Floating-point registers: `XMM4`-`XMM15`

### Example Assembly Function Declaration

Here’s an example of a simple function in assembly that uses these conventions:

```assembly
section .text
global my_function

my_function:
    ; Prologue
    push rbp
    mov rbp, rsp
    sub rsp, 32              ; Reserve shadow space

    ; Function body
    mov rax, rcx             ; Do something with the first parameter (passed in RCX)

    ; Epilogue
    add rsp, 32              ; Restore stack
    pop rbp
    ret
```

## Summary

- **Registers for Arguments**: The first four integer/pointer arguments use RCX, RDX, R8, R9; floating-point arguments use XMM0-XMM3.
- **Return Register**: RAX for integer/pointer/structs up to 8 bytes, XMM0 for floating-point.
- **Preserved Registers**: RBX, RBP, RDI, RSI, R12-R15.
- **Volatile Registers**: RAX, RCX, RDX, R8-R11, XMM4-XMM15.
- **Shadow Space**: 32 bytes of shadow space on the stack reserved for the callee.
- **Stack Alignment**: Stack must be 16-byte aligned at the point of the call instruction.

## Register usage in jitBrainsForth.

jitBrainsForth uses the x64 calling convention for Windows, ensuring that functions adhere to the register usage and stack alignment requirements. This allows for efficient function calls and interoperability with other code that follows the same conventions.

However jBF does use some registers for its own purposes

These are the data stack DS, the return/temporary stack RS, the dictionary pointer H, and the locals stack LS.
These values persist across function calls and are used to manage the state of the interpreter 
and track the current execution context.

The register allocated to DS is R15.
The register allocated to RS is R14.
The register allocated to LS is R13.

These are used from C and FORTH code to access the stacks.

Three scratch registers are used to perform stack operations 

Scratch S1 is RAX
Scratch S2 is RCX
Scratch S3 is RDX

Arguments passed to c calls are passed in A1, A2, A3    

A1 is RCX
A2 is RDX
A3 is R8

These register definitions are allocated in ASMJIT code like this

```cpp
    auto& a = *context.assembler;
    asmjit::x86::Gp ds = asmjit::x86::r11;
    asmjit::x86::Gp rs = asmjit::x86::r10;
    asmjit::x86::Gp ls = asmjit::x86::r9;
    asmjit::x86::Gp s1 = asmjit::x86::rax;
    asmjit::x86::Gp s2 = asmjit::x86::rcx;
    asmjit::x86::Gp s3 = asmjit::x86::rdx;
    asmjit::x86::Gp a1 = asmjit::x86::rcx;
    asmjit::x86::Gp a2 = asmjit::x86::rdx;
    asmjit::x86::Gp a3 = asmjit::x86::r8;
```
    
The DS, RS, registers are pointers to full descending stacks.

Convenience functions exist to push and pop 64 bit values to and from these stacks.

```cpp
    void pushDS(asmjit::x86::Gp reg) {
        auto& a = *context.assembler;
        a.mov(asmjit::x86::qword_ptr(ds, 0), reg);
        a.sub(ds, 8);
    }

    void popDS(asmjit::x86::Gp reg) {
        auto& a = *context.assembler;
        a.add(ds, 8);
        a.mov(reg, asmjit::x86::qword_ptr(ds, 0));
    }

    void pushRS(asmjit::x86::Gp reg) {
        auto& a = *context.assembler;
        a.mov(asmjit::x86::qword_ptr(rs, 0), reg);
        a.sub(rs, 8);
    }

    void popRS(asmjit::x86::Gp reg) {
        auto& a = *context.assembler;
        a.add(rs, 8);
        a.mov(reg, asmjit::x86::qword_ptr(rs, 0));
    }
```

The LS register is a pointer to a full descending stack of local variables.

Local variables are accessed by an offset from the LS register.
Fetch local variable from the LS stack at offset and push it to DS
Or pop DS and place the value on the LS stack at the offset
    
    ```cpp
        void fetchLocal(asmjit::x86::Gp reg, int offset) {
            auto& a = *context.assembler;
            a.mov(reg, asmjit::x86::qword_ptr(ls, offset));
            pushDS(reg);
        }
    
        void storeLocal(asmjit::x86::Gp reg, int offset) {
            auto& a = *context.assembler;
            popDS(reg);
            a.mov(asmjit::x86::qword_ptr(ls, offset), reg);
        }
    ``` 
 
At run time each FORTH word allocates space on the LS stack for its local variables,
by decrementing the LS register by the number of local variables times 8.

```cpp
    void allocateLocals(int count) {
        auto& a = *context.assembler;
        a.sub(ls, count * 8);
    }
```

The dictionary pointer H is a pointer to the current word in the dictionary.
The dictionary is a linked list of words, each word containing a pointer to the previous word in the dictionary.

The dictionary pointer is used to access the current word's name, function pointer, and other metadata.
H changes when words are added or forgotten from the dictionary.

 
    
  
