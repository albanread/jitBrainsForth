#include "X86_64Assembler.h" // Include assembler header
#include <iostream>          // For debugging or example usage

namespace jitBrainsForth
{
    // Write the immediate value to the machine code
    void X86_64Assembler::writeImmediateValue(uint64_t value, uint8_t size, std::vector<uint8_t>& machineCode)
    {
        for (uint8_t i = 0; i < size; ++i)
        {
            machineCode.push_back(static_cast<uint8_t>(value & 0xFF));
            value >>= 8;
        }
    }

    // Parse operands from strings
    Operand X86_64Assembler::parseOperand(const std::string& operand)
    {
        using namespace AssemblerUtils; // Inject utility namespace for ease of use

        if (std::isdigit(operand[0]) || operand[0] == '$' || operand.starts_with("0x"))
        {
            return Operand(OperandType::IMMEDIATE, operand);
        }
        if (registers.find(operand) != registers.end())
        {
            return Operand(OperandType::REGISTER, operand);
        }
        if (operand.front() == '[' && operand.back() == ']')
        {
            return Operand(OperandType::MEMORY, operand.substr(1, operand.size() - 2));
        }
        throw AssemblerException("Invalid operand: " + operand);
    }

    // Parse and encode a memory operand
    void X86_64Assembler::encodeMemoryOperand(const std::string& operand, uint8_t& modRM,
                                              std::vector<uint8_t>& machineCode)
    {
        // Check for a simple memory operand like `[rbx + 0x10]`
        std::string baseRegister = "rbx"; // Example base
        uint32_t displacement = 0x10; // Example displacement

        if (!baseRegister.empty())
        {
            modRM |= 0x80; // Mod field (0x80 = 32-bit displacement)
            machineCode.push_back(displacement & 0xFF);
            machineCode.push_back((displacement >> 8) & 0xFF);
            machineCode.push_back((displacement >> 16) & 0xFF);
            machineCode.push_back((displacement >> 24) & 0xFF);
        }
    }


    // Assemble a single instruction
    void X86_64Assembler::assembleInstruction(const ParsedInstruction& instruction, std::vector<uint8_t>& machineCode)
    {
        using namespace AssemblerUtils;

        auto it = mnemonics.find(instruction.mnemonic);
        if (it == mnemonics.end())
        {
            throw AssemblerException("Unknown mnemonic: " + instruction.mnemonic);
        }
        uint8_t opcode = it->second;

        // Handle LEA instruction
        if (instruction.mnemonic == "LEA" && instruction.operands.size() == 2)
        {
            const auto& dest = instruction.operands[0];
            const auto& src = instruction.operands[1];


            if (dest.type == OperandType::REGISTER && src.type == OperandType::MEMORY)
            {
                // Step 1: Write REX prefix for 64-bit register or extended register
                uint8_t rex = 0x40; // Base REX prefix
                if (registers.at(dest.value) >= 8 || registers.at(src.value) >= 8)
                {
                    rex |= 0x08; // REX.W for 64-bit width
                }
                if (registers.at(dest.value) >= 8)
                {
                    rex |= 0x04; // REX.R
                }
                if (registers.at(src.value) >= 8)
                {
                    rex |= 0x01; // REX.B
                }
                if (rex != 0x40)
                {
                    machineCode.push_back(rex);
                }


                // Step 2: Write the LEA opcode
                machineCode.push_back(opcode);

                // Step 3: Encode ModR/M byte
                uint8_t modRM = 0;
                modRM |= (0b11 << 6); // Mod: Register addressing mode (memory operand)
                modRM |= ((registers.at(dest.value) & 0x07) << 3); // Reg: Destination register
                modRM |= (registers.at(src.value) & 0x07); // R/M: Source register

                machineCode.push_back(modRM);
                // Step 4: (Optional) Encode displacement or SIB
                // For now, assume simple memory operands (e.g., `[rbx + 0x10]` will be handled later).
                return;
            }
            else
            {
                throw AssemblerException("LEA expects a register and memory operand");
            }
        }

        if (instruction.mnemonic == "MOV" && instruction.operands.size() == 2)
        {
            auto dest = instruction.operands[0];
            auto src = instruction.operands[1];

            if (dest.type == OperandType::REGISTER && src.type == OperandType::IMMEDIATE)
            {
                uint64_t immValue = parseImmediateValue(src.value);

                if (registers.at(dest.value) >= 8)
                {
                    machineCode.push_back(0x48); // REX.W for 64-bit ops
                }

                machineCode.push_back(0xB8 + (registers.at(dest.value) & 0x07));
                writeImmediateValue(immValue, 8, machineCode);
                return;
            }
        }

         machineCode.push_back(opcode);

    }

    // Main assembler logic
    void X86_64Assembler::assemble(const std::string& assemblySource, std::vector<uint8_t>& machineCode)
    {
        using namespace AssemblerUtils;

        machineCode.clear();
        auto lines = split(assemblySource, '\n'); // Use utility `split` function

        for (const auto& line : lines)
        {
            std::string trimmedLine = trim(line); // Use utility `trim` function
            if (trimmedLine.empty()) continue;

            auto tokens = split(trimmedLine, ' ');
            std::string mnemonic = tokens[0];
            std::vector<Operand> operands;

            if (tokens.size() > 1)
            {
                auto operandsText = split(tokens[1], ',');
                for (const auto& op : operandsText)
                {
                    operands.push_back(parseOperand(op));
                }
            }

            ParsedInstruction instruction{mnemonic, operands};
            assembleInstruction(instruction, machineCode);
        }
    }
} // namespace jitBrainsForth
