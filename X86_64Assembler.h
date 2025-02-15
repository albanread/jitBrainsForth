#ifndef X86_64ASSEMBLER_H
#define X86_64ASSEMBLER_H

#include "AssemblerUtils.h" // Include the utility header
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstdint> // For uint8_t, uint32_t, etc.

namespace jitBrainsForth
{
    // Exception class for assembler errors
    class AssemblerException : public std::runtime_error
    {
    public:
        explicit AssemblerException(const std::string& message)
            : std::runtime_error("Assembler Error: " + message)
        {
        }
    };

    // Represents different operand types (register, immediate, memory, etc.)
    enum class OperandType { REGISTER, IMMEDIATE, MEMORY, UNDEFINED };

    // Struct for operand representation
    struct Operand
    {
        OperandType type;
        std::string value; // Text representation
        Operand(OperandType t, const std::string& v) : type(t), value(v)
        {
        }
    };

    // Struct for parsed instruction representation
    struct ParsedInstruction
    {
        std::string mnemonic;
        std::vector<Operand> operands;
    };

    // The x86-64 Assembler class
    class X86_64Assembler
    {
    private:
        // Supported mnemonics with opcodes
        static inline const std::unordered_map<std::string, uint8_t> mnemonics = {
            {"ADD", 0x01},
            {"SUB", 0x29},
            {"MOV", 0xB8}, // MOV with immediate value to register
            {"NOP", 0x90},
            {"LEA", 0x8D} // Opcode for LEA
        };

        static inline const std::unordered_map<std::string, uint8_t> registers = {
            // General-purpose registers (64-bit)
            {"rax", 0}, {"rbx", 3}, {"rcx", 1}, {"rdx", 2},
            {"rsi", 6}, {"rdi", 7}, {"rsp", 4}, {"rbp", 5},
            {"r8", 8}, {"r9", 9}, {"r10", 10}, {"r11", 11},
            {"r12", 12}, {"r13", 13}, {"r14", 14}, {"r15", 15},

            // General-purpose registers (32-bit, compatibility)
            {"eax", 0}, {"ebx", 3}, {"ecx", 1}, {"edx", 2},
            {"esi", 6}, {"edi", 7}, {"esp", 4}, {"ebp", 5},
            {"r8d", 8}, {"r9d", 9}, {"r10d", 10}, {"r11d", 11},
            {"r12d", 12}, {"r13d", 13}, {"r14d", 14}, {"r15d", 15},

            // General-purpose registers (16-bit)
            {"ax", 0}, {"bx", 3}, {"cx", 1}, {"dx", 2},
            {"si", 6}, {"di", 7}, {"sp", 4}, {"bp", 5},
            {"r8w", 8}, {"r9w", 9}, {"r10w", 10}, {"r11w", 11},
            {"r12w", 12}, {"r13w", 13}, {"r14w", 14}, {"r15w", 15},

            // General-purpose registers (8-bit)
            {"al", 0}, {"bl", 3}, {"cl", 1}, {"dl", 2},
            {"sil", 6}, {"dil", 7}, {"spl", 4}, {"bpl", 5},
            {"r8b", 8}, {"r9b", 9}, {"r10b", 10}, {"r11b", 11},
            {"r12b", 12}, {"r13b", 13}, {"r14b", 14}, {"r15b", 15},

            // 8-bit legacy registers (from the low byte of AX, BX, etc.)
            {"ah", 4}, {"bh", 7}, {"ch", 5}, {"dh", 6}
        };


        // Write the immediate value to the machine code
        void writeImmediateValue(uint64_t value, uint8_t size, std::vector<uint8_t>& machineCode);

        // Parse operands from strings
        Operand parseOperand(const std::string& operand);

        static void encodeMemoryOperand(const std::string& operand, uint8_t& modRM, std::vector<uint8_t>& machineCode);

        // Assemble a single instruction
        void assembleInstruction(const ParsedInstruction& instruction, std::vector<uint8_t>& machineCode);

    public:
        // Public method to assemble assembly source into machine code
        void assemble(const std::string& assemblySource, std::vector<uint8_t>& machineCode);
    };
} // namespace jitBrainsForth

#endif // X86_64ASSEMBLER_H
