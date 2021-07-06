#include "cpu.h"

#include <stdexcept>

Cpu::Cpu(Memory& memory): m_Memory(memory)
{
}

void Cpu::StartExecution()
{
    this->pc = GB_ROM_ENTRY_POINT;

    while(1)
    {
        this->ExecuteInstruction();
    }
}

void Cpu::ExecuteInstruction()
{
    uint8_t op = m_Memory.ReadMemory8(pc);

    switch(op)
    {
    case 0x00: // NOP
        pc += 1;
        break;
    case 0x06: // LD B, n
    case 0x0E: // LD C, n
    case 0x16: // LD D, n
    case 0x1E: // LD E, n
    case 0x26: // LD H, n
    case 0x2E: // LD L, n
        // 8 cycles
        Execute_Load_8(op);
        break;
    case 0x3A: // LDD A, (HL)
    case 0x32: // LDD A, (HL)
        // 8 cycles
        Execute_Load_HL_A_Dec(op);
        break;
    case 0x20: // JR NZ
    case 0x28: // JR Z
    case 0x30: // JR NC
    case 0x38: // JR C
        // 8 cycles
        Execute_Jr(op);
        break;
    case 0x21: // LD HL, nn
        // 12 cycles
        hl.both = m_Memory.ReadMemory16(pc + 1);
        pc += 3;
        break;
    case 0x0B: // DEC BC
    case 0x1B: // DEC DE
    case 0x2B: // DEC HL
    case 0x3B: // DEC SP
        Execute_Dec_16(op);
        break;
    case 0x3D: // DEC A
    case 0x05: // DEC B
    case 0x0D: // DEC C
    case 0x15: // DEC D
    case 0x1D: // DEC E
    case 0x25: // DEC H
    case 0x2D: // DEC L
        // 4 cycles
        Execute_Dec_8(op);
        break;
    case 0xAF: // XOR A
    case 0xA8: // XOR B
    case 0xA9: // XOR C
    case 0xAA: // XOR D
    case 0xAB: // XOR E
    case 0xAC: // XOR H
    case 0xAD: // XOR L
        // 4 cycles
        Execute_Xor_N(op);
        break;
    case 0xC3: // JP 16bit
        // 12 cycles
        pc = m_Memory.ReadMemory16(pc + 1);
        break;
    default:
        throw std::runtime_error("Not implemented " + op);
    }
}

void Cpu::Execute_Xor_N(uint8_t opCode)
{
    // 4 cycles
    uint8_t operand;
    switch(opCode)
    {
    case 0xAF: // XOR A
        operand = af.first;
        break;
    case 0xA8: // XOR B
        operand = bc.first;
        break;
    case 0XA9: // XOR C
        operand = bc.second;
        break;
    case 0XAA: // XOR D
        operand = de.first;
        break;
    case 0xAB: // XOR E
        operand = de.second;
        break;
    case 0xAC: // XOR H
        operand = hl.first;
        break;
    case 0xAD: // XOR L
        operand = hl.second;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    af.first ^= operand;
    flags.z = af.first == 0x0;
    flags.n = false;
    flags.h = false;
    flags.c = false;

    pc += 1;
}

void Cpu::Execute_Load_8(uint8_t opCode)
{
    // 8 cycles
    uint8_t* operand = nullptr;
    switch(opCode)
    {
    case 0x06: // LD B, n
        operand = &bc.first;
        break;
    case 0x0E: // LD C, n
        operand = &bc.second;
        break;
    case 0x16: // LD D, n
        operand = &de.first;
        break;
    case 0x1E: // LD E, n
        operand = &de.second;
        break;
    case 0x26: // LD H, n
        operand = &hl.first;
        break;
    case 0x2E: // LD L, n
        operand = &hl.second;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }
    *operand = m_Memory.ReadMemory8(pc + 1);

    pc += 2;
}

void Cpu::Execute_Load_HL_A_Dec(uint8_t opCode)
{
    if(opCode != 0x3A && opCode != 0x32)
        throw std::runtime_error("Unimplemented opCode " + opCode);

    af.first = m_Memory.ReadMemory8(hl.both);
    Execute_Dec_16(0x2B, true); // DEC HL val
    pc += 1;
}

void Cpu::Execute_Dec_8(uint8_t opCode)
{
    uint8_t* operand = nullptr;
    bool hl_ref = false;
    switch (opCode) 
    {
    // 4 cycles
    case 0x3D: // DEC A
        operand = &af.first;
        break;
    case 0x05: // DEC B
        operand = &bc.first;
        break;
    case 0x0D: // DEC C
        operand = &bc.second;
        break;
    case 0x15: // DEC D
        operand = &de.first;
        break;
    case 0x1D: // DEC E
        operand = &de.second;
        break;
    case 0x25: // DEC H
        operand = &hl.first;
        break;
    case 0x2D: // DEC L
        operand = &hl.second;
        break;
    // 12 cycles
    case 0x35: // DEC (HL)
        hl_ref = true;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    uint8_t orig_val;
    uint8_t new_val;
    if(hl_ref)
    {
        orig_val = m_Memory.ReadMemory8(hl.both);
        new_val = orig_val - 1;
        m_Memory.SetMemory8(hl.both, new_val);
    }
    else
    {
        orig_val = *operand;
        *operand -= 1;
        new_val = *operand;
    }
    

    flags.z = new_val == 0;
    flags.n = true;
    flags.h = HalfCarryOnSubtraction(orig_val, new_val);

    pc += 1;
}

void Cpu::Execute_Dec_16(uint8_t opCode, bool suppress_pc_inc)
{
    uint16_t* operand = nullptr;
    switch(opCode)
    {
    case 0x0B: // DEC BC
        operand = &bc.both;
        break;
    case 0x1B: // DEC DE
        operand = &de.both;
        break;
    case 0x2B: // DEC HL
        operand = &hl.both;
        break;
    case 0x3B: // DEC SP
        operand = &sp;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    *operand -= 1;

    if (!suppress_pc_inc)
        pc += 1;
}

void Cpu::Execute_Jr(uint8_t opCode)
{
    // 8 cycles
    bool jp = false;
    switch(opCode)
    {
    case 0x20: // JR NZ
        jp = !flags.z;
        break;
    case 0x28: // JR Z
        jp = flags.z;
        break;
    case 0x30: // JR NC
        jp = !flags.c;
        break;
    case 0x38: // JR C
        jp = flags.c;
        break;
    }

    if(!jp)
    {
        pc += 2;
        return;
    }
    uint8_t orig_val = m_Memory.ReadMemory8(pc + 1);
    int8_t jump_relative = static_cast<int8_t>(orig_val);
    pc += jump_relative;
}
