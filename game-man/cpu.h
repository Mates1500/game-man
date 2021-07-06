#pragma once
#include "memory.h"
#define GB_ROM_ENTRY_POINT 0x100

class Cpu
{
public:
    Cpu(Memory& memory);
    void StartExecution();
    void ExecuteInstruction();
private:
    // kindly copypasted from https://stackoverflow.com/a/57822729
    static constexpr bool HalfCarryOnAddition(uint8_t first_num, uint8_t second_num)
    {
        return (((first_num & 0x0F) + (second_num & 0x0F)) & 0x10) == 0x10;
    }

    static constexpr bool HalfCarryOnAddition(uint16_t first_num, uint16_t second_num)
    {
        return (((first_num & 0x00FF) + (second_num & 0x00FF)) & 0x0100) == 0x0100;
    }

    static constexpr bool HalfCarryOnSubtraction(uint8_t first_num, uint8_t second_num)
    {
        return (int)(first_num & 0x0F) - (int)(second_num & 0x0F) < 0;
    }

    static constexpr bool HalfCarryOnSubtraction(uint16_t first_num, uint16_t second_num)
    {
        return (int)(first_num & 0x00FF) - (int)(second_num & 0x00FF) < 0;
    }

    void Execute_Xor_N(uint8_t opCode);
    void Execute_Load_8(uint8_t opCode);
    void Execute_Load_HL_A_Dec(uint8_t opCode);
    void Execute_Dec_8(uint8_t opCode);
    void Execute_Dec_16(uint8_t opCode, bool suppress_pc_inc = false);
    void Execute_Jr(uint8_t opCode);
    Memory& m_Memory;

    union TwinRegister
    {
        struct
        {
            uint8_t first;
            uint8_t second;
        };
        uint16_t both;
    };

    TwinRegister af; // A and F registers, A is accumulator, F is flag register
    TwinRegister bc; // B and C registers
    TwinRegister de; // D and E registers
    TwinRegister hl; // H and L registers, often used to point to addresses in memory
    uint16_t sp; // stack pointer register
    uint16_t pc; // program counter

    struct cpu_flags
    {
        bool z;
        bool n;
        bool h;
        bool c;
    };
    cpu_flags flags;
};

