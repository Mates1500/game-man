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

    static constexpr uint8_t Swap(uint8_t val)
    {
        return (val >> 4) | (val << 4);
    }

    static constexpr bool CarryOnAddition(uint16_t first_num, uint16_t second_num)
    {
        // is this garbage and slow? probably, but it works
        return static_cast<uint32_t>(first_num) + static_cast<uint32_t>(second_num) > 0xFFFF;
    }

    static constexpr bool CarryOnAddition(uint8_t first_num, uint8_t second_num)
    {
        return static_cast<uint16_t>(first_num) + static_cast<uint16_t>(second_num) > 0xFF;
    }

    static constexpr bool CarryOnSubtraction(uint16_t first_num, uint16_t second_num)
    {
        return static_cast<int32_t>(first_num) - static_cast<int32_t>(second_num) < 0;
    }

    static constexpr bool CarryOnSubtraction(uint8_t first_num, uint8_t second_num)
    {
        return static_cast<int16_t>(first_num) - static_cast<int16_t>(second_num) < 0;
    }

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

    void ElapseCycles(int cycles);

    void Execute_Xor_N(uint8_t opCode);
    void Execute_Load_8_Val(uint8_t opCode);
    void Execute_Load_8_Operand(uint8_t opCode);
    void Execute_Load_16_Val(uint8_t opCode);
    void Execute_Load_HL_A_Dec(uint8_t opCode);
    void Execute_Load_HL_A_Inc();
    void Execute_Load_FF00_C_A();
    void Execute_Dec_8(uint8_t opCode);
    void Execute_Dec_16(uint8_t opCode, bool suppress_pc_inc = false);
    void Execute_Jr_Flag(uint8_t opCode);
    void Execute_Jr_n(uint8_t opCode);
    void Execute_Jp_HL();
    void Execute_Add_HL_Operand(uint8_t opCode);
    void Execute_Add_8(uint8_t opCode);
    void Execute_Inc_8(uint8_t opCode);
    void Execute_Inc_16(uint8_t opCode, bool suppress_pc_inc = false);
    void Execute_Sub_8(uint8_t opCode);
    void Execute_SBC_8(uint8_t opCode);
    void Execute_LDH_n_A();
    void Execute_LDH_A_n();
    void Execute_Compare_8(uint8_t opCode);
    void Execute_Or_N(uint8_t opCode);
    void Execute_And_N(uint8_t opCode);
    void Execute_Swap();
    void Execute_Rst(uint8_t opCode);

    uint16_t PopStack();
    void PushStack(uint16_t val);

    void Execute_Call();
    void Execute_Return(uint8_t opCode);
    void Execute_Pop(uint8_t opCode);
    void Execute_Push(uint8_t opCode);

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
    bool interrupts_enabled;
};

