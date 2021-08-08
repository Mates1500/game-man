#include "cpu.h"

#include <stdexcept>

Cpu::Cpu(Memory& memory): m_Memory(memory)
{
    this->sp = SP_INIT_VAL;
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
    case 0x03: // INC BC
    case 0x13: // INC DE
    case 0x23: // INC HL
    case 0x33: // INC SP
        Execute_Inc_16(op);
        break;
    case 0x3C: // INC A
    case 0x04: // INC B
    case 0x0C: // INC C
    case 0x14: // INC D
    case 0x1C: // INC E
    case 0x24: // INC H
    case 0x2C: // INC L
    case 0x34: // INC (HL)
        Execute_Inc_8(op);
        break;
    case 0x09: // ADD HL, BC
    case 0x19: // ADD HL, DE
    case 0x29: // ADD HL, HL
    case 0x39: // ADD HL, SP
        Execute_Add_HL_Operand(op);
        break;
    case 0x97: // SUB A
    case 0x90: // SUB B
    case 0x91: // SUB C
    case 0x92: // SUB D
    case 0x93: // SUB E
    case 0x94: // SUB H
    case 0x95: // SUB L
    case 0x96: // SUB (HL)
    case 0xD6: // SUB # ???
        // 4 cycles for most
        // 8 cycles for HL, #
        Execute_Sub_8(op);
        break;
    case 0x9F: // SBC A, A
    case 0x98: // SBC A, B
    case 0x99: // SBC A, C
    case 0x9A: // SBC A, D
    case 0x9B: // SBC A, E
    case 0x9C: // SBC A, H
    case 0x9D: // SBC A, L
    case 0x9E: // SBC A, (HL)
        // 4 cycles for most
        // 8 Cycles for HL
        Execute_SBC_8(op);
        break;
    case 0x2F: // CPL
        // one's complement of A register
        // 4 cycles
        af.first = ~af.first;

        flags.n = true;
        flags.h = true;
        pc += 1;
        break;
    case 0x22: // LDI (HL), A
        // 8 cycles
        m_Memory.SetMemory8(hl.both, af.first);
        ++hl.both;
        pc += 1;
        break;
    case 0x3E: // LD A, #
    case 0x06: // LD B, n
    case 0x0E: // LD C, n
    case 0x16: // LD D, n
    case 0x1E: // LD E, n
    case 0x26: // LD H, n
    case 0x2E: // LD L, n
        // 8 cycles
        Execute_Load_8_Val(op);
        break;
    case 0x01: // LD BC, nn
    case 0x11: // LD DE, nn
    case 0x21: // LD HL, nn
    case 0x31: // LD SP, nn
        // 12 cycles
        Execute_Load_16_Val(op);
        break;
    case 0x7F: // LD A, A
    case 0x78: // LD A, B
    case 0x79: // LD A, C
    case 0x7A: // LD A, D
    case 0x7B: // LD A, E
    case 0x7C: // LD A, H
    case 0x7D: // LD A, L
    case 0x7E: // LD A, (HL)
    case 0x47: // LD B, A
    case 0x40: // LD B, B
    case 0x41: // LD B, C
    case 0x42: // LD B, D
    case 0x43: // LD B, E
    case 0x44: // LD B, H
    case 0x45: // LD B, L
    case 0x46: // LD B, (HL)
    case 0x4F: // LD C, A
    case 0x48: // LD C, B
    case 0x49: // LD C, C
    case 0x4A: // LD C, D
    case 0x4B: // LD C, E
    case 0x4C: // LD C, H
    case 0x4D: // LD C, L
    case 0x4E: // LD C, (HL)
    case 0x57: // LD D, A
    case 0x50: // LD D, B
    case 0x51: // LD D, C
    case 0x52: // LD D, D
    case 0x53: // LD D, E
    case 0x54: // LD D, H
    case 0x55: // LD D, L
    case 0x56: // LD D, (HL)
    case 0x5F: // LD E, A
    case 0x58: // LD E, B
    case 0x59: // LD E, C
    case 0x5A: // LD E, D
    case 0x5B: // LD E, E
    case 0x5C: // LD E, H
    case 0x5D: // LD E, L
    case 0x5E: // LD E, (HL)
    case 0x67: // LD H, A
    case 0x60: // LD H, B
    case 0x61: // LD H, C
    case 0x62: // LD H, D
    case 0x63: // LD H ,E
    case 0x64: // LD H, H
    case 0x65: // LD H, L
    case 0x66: // LD H, (HL)
    case 0x6F: // LD L, A
    case 0x68: // LD L, B
    case 0x69: // LD L, C
    case 0x6A: // LD L, D
    case 0x6B: // LD L, E
    case 0x6C: // LD L, H
    case 0x6D: // LD L, L
    case 0x6E: // LD L, (HL)
    case 0x70: // LD (HL), B
    case 0x71: // LD (HL), C
    case 0x72: // LD (HL), D
    case 0x73: // LD (HL), E
    case 0x74: // LD (HL), H
    case 0x75: // LD (HL), L
    case 0x02: // LD (BC), A
    case 0x12: // LD (DE), A
    case 0x77: // LD (HL), A
    case 0x36: // LD (HL), n
    case 0xEA:
        // 4 cycles for most
        // 8 or 12 cycles for anything using 16 bit stuff
        Execute_Load_8_Operand(op);
        break;
    case 0x3A: // LDD A, (HL)
    case 0x32: // LDD A, (HL)
        // 8 cycles
        Execute_Load_HL_A_Dec(op);
        break;
    case 0x2A: // LDI A, (HL)
        Execute_Load_HL_A_Inc();
        break;
    case 0xE2: // LD (C), A
        Execute_Load_FF00_C_A();
        break;
    case 0x20: // JR NZ
    case 0x28: // JR Z
    case 0x30: // JR NC
    case 0x38: // JR C
        // 8 cycles
        Execute_Jr_Flag(op);
        break;
    case 0x18: // JR n
        Execute_Jr_n(op);
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
    case 0xE0: // LDH (n), A
        // 12 cycles
        Execute_LDH_n_A();
        break;
    case 0xF0: // LDH A, (n)
        Execute_LDH_A_n();
        break;
    case 0xFB: // EI
        // 4 cycles
        interrupts_enabled = true;
        pc += 1;
        break;
    case 0xF3: // DI
        // 4 cycles
        interrupts_enabled = false;
        pc += 1;
        break;
    case 0xBF: // CP A
    case 0xB8: // CP B
    case 0xB9: // CP C
    case 0xBA: // CP D
    case 0xBB: // CP E
    case 0xBC: // CP H
    case 0xBD: // CP L
    case 0xBE: // CP (HL)
    case 0xFE: // CP #
        Execute_Compare_8(op);
        break;
    case 0xCD: // CALL nn
        Execute_Call();
        break;
    case 0xC9: // RET
    case 0xC0: // RET NZ
    case 0xC8: // RET Z
    case 0xD0: // RET NC
    case 0xD8: // RET C
        Execute_Return(op);
        break;
    case 0xB7: // OR A
    case 0xB0: // OR B
    case 0xB1: // OR C
    case 0xB2: // OR D
    case 0xB3: // OR E
    case 0xB4: // OR H
    case 0xB5: // OR L
    case 0xB6: // OR (HL)
    case 0xF6: // OR (#)
        Execute_Or_N(op);
        break;
    case 0xA7: // AND A
    case 0xA0: // AND B
    case 0xA1: // AND C
    case 0xA2: // AND D
    case 0xA3: // AND E
    case 0xA4: // AND H
    case 0xA5: // AND L
    case 0xA6: // AND (HL)
    case 0xE6: // AND #
        Execute_And_N(op);
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

void Cpu::Execute_Load_8_Val(uint8_t opCode)
{
    // 8 cycles
    uint8_t* operand = nullptr;
    switch(opCode)
    {
    case 0x3E: // LD A, #
        operand = &af.first;
        break;
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

void Cpu::Execute_Load_8_Operand(uint8_t opCode)
{
    uint8_t* operandDest = nullptr;
    uint8_t srcVal = 0;
    uint8_t cycles = 4; // most operations, HL related use 8, n load uses 12
    uint8_t jp_counter = 1;
    switch(opCode)
    {
    case 0x7F: // LD A, A
        operandDest = &af.first;
        srcVal = af.first;
        break;
    case 0x78: // LD A, B
        operandDest = &af.first;
        srcVal = bc.first;
        break;
    case 0x79: // LD A, C
        operandDest = &af.first;
        srcVal = bc.second;
        break;
    case 0x7A: // LD A, D
        operandDest = &af.first;
        srcVal = de.first;
        break;
    case 0x7B: // LD A, E
        operandDest = &af.first;
        srcVal = de.second;
        break;
    case 0x7C: // LD A, H
        operandDest = &af.first;
        srcVal = hl.first;
        break;
    case 0x7D: // LD A, L
        operandDest = &af.first;
        srcVal = hl.second;
        break;
    case 0x47: // LD B, A
        operandDest = &bc.first;
        srcVal = af.first;
        break;
    case 0x40: // LD B, B
        operandDest = &bc.first;
        srcVal = bc.first;
        break;
    case 0x41: // LD B, C
        operandDest = &bc.first;
        srcVal = bc.second;
        break;
    case 0x42: // LD B, D
        operandDest = &bc.first;
        srcVal = de.first;
        break;
    case 0x43: // LD B, E
        operandDest = &bc.first;
        srcVal = de.second;
        break;
    case 0x44: // LD B, H
        operandDest = &bc.first;
        srcVal = hl.first;
        break;
    case 0x45: // LD B, L
        operandDest = &bc.first;
        srcVal = hl.second;
        break;
    case 0x4F: // LD C, A
        operandDest = &bc.second;
        srcVal = af.first;
        break;
    case 0x48: // LD C, B
        operandDest = &bc.second;
        srcVal = bc.first;
        break;
    case 0x49: // LD C, C
        operandDest = &bc.second;
        srcVal = bc.second;
        break;
    case 0x4A: // LD C, D
        operandDest = &bc.second;
        srcVal = de.first;
        break;
    case 0x4B: // LD C, E
        operandDest = &bc.second;
        srcVal = de.second;
        break;
    case 0x4C: // LD C, H
        operandDest = &bc.second;
        srcVal = hl.first;
        break;
    case 0x4D: // LD C, L
        operandDest = &bc.second;
        srcVal = hl.second;
        break;
    case 0x57: // LD D, A
        operandDest = &de.first;
        srcVal = af.first;
        break;
    case 0x50: // LD D, B
        operandDest = &de.first;
        srcVal = bc.first;
        break;
    case 0x51: // LD D, C
        operandDest = &de.first;
        srcVal = bc.second;
        break;
    case 0x52: // LD D, D
        operandDest = &de.first;
        srcVal = de.first;
        break;
    case 0x53: // LD D, E
        operandDest = &de.first;
        srcVal = de.second;
        break;
    case 0x54: // LD D, H
        operandDest = &de.first;
        srcVal = hl.first;
        break;
    case 0x55: // LD D, L
        operandDest = &de.first;
        srcVal = hl.second;
        break;
    case 0x5F: // LD E, A
        operandDest = &de.second;
        srcVal = bc.first;
        break;
    case 0x58: // LD E, B
        operandDest = &de.second;
        srcVal = bc.first;
        break;
    case 0x59: // LD E, C
        operandDest = &de.second;
        srcVal = bc.second;
        break;
    case 0x5A: // LD E, D
        operandDest = &de.second;
        srcVal = de.first;
        break;
    case 0x5B: // LD E, E
        operandDest = &de.second;
        srcVal = de.second;
        break;
    case 0x5C: // LD E, H
        operandDest = &de.second;
        srcVal = hl.first;
        break;
    case 0x5D: // LD E, L
        operandDest = &de.second;
        srcVal = hl.second;
        break;
    case 0x67: // LD H, A
        operandDest = &hl.first;
        srcVal = af.first;
        break;
    case 0x60: // LD H, B
        operandDest = &hl.first;
        srcVal = bc.first;
        break;
    case 0x61: // LD H, C
        operandDest = &hl.first;
        srcVal = bc.second;
        break;
    case 0x62: // LD H, D
        operandDest = &hl.first;
        srcVal = de.first;
        break;
    case 0x63: // LD H ,E
        operandDest = &hl.first;
        srcVal = de.second;
        break;
    case 0x64: // LD H, H
        operandDest = &hl.first;
        srcVal = hl.first;
        break;
    case 0x65: // LD H, L
        operandDest = &hl.first;
        srcVal = hl.second;
        break;
    case 0x6F: // LD L, A
        operandDest = &hl.second;
        srcVal = af.first;
        break;
    case 0x68: // LD L, B
        operandDest = &hl.second;
        srcVal = bc.first;
        break;
    case 0x69: // LD L, C
        operandDest = &hl.second;
        srcVal = bc.second;
        break;
    case 0x6A: // LD L, D
        operandDest = &hl.second;
        srcVal = de.first;
        break;
    case 0x6B: // LD L, E
        operandDest = &hl.second;
        srcVal = de.second;
        break;
    case 0x6C: // LD L, H
        operandDest = &hl.second;
        srcVal = hl.first;
        break;
    case 0x6D: // LD L, L
        operandDest = &hl.second;
        srcVal = hl.second;
        break;
    case 0x7E: // LD A, (HL)
        operandDest = &af.first;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x46: // LD B, (HL)
        operandDest = &bc.first;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x4E: // LD C, (HL)
        operandDest = &bc.second;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x56: // LD D, (HL)
        operandDest = &de.first;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x5E: // LD E, (HL)
        operandDest = &de.second;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x66: // LD H, (HL)
        operandDest = &hl.first;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x6E: // LD L, (HL)
        operandDest = &hl.second;
        srcVal = this->m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0x70: // LD (HL), B
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = bc.first;
        cycles = 8;
        break;
    case 0x71: // LD (HL), C
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = bc.second;
        cycles = 8;
        break;
    case 0x72: // LD (HL), D
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = de.first;
        cycles = 8;
        break;
    case 0x73: // LD (HL), E
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = de.second;
        cycles = 8;
        break;
    case 0x74: // LD (HL), H
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = hl.first;
        cycles = 8;
        break;
    case 0x75: // LD (HL), L
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = hl.second;
        cycles = 8;
        break;
    case 0x02: // LD (BC), A
        operandDest = this->m_Memory.GetPtrAt(bc.both);
        srcVal = af.first;
        cycles = 8;
        break;
    case 0x12: // LD (DE), A
        operandDest = this->m_Memory.GetPtrAt(de.both);
        srcVal = af.first;
        cycles = 8;
        break;
    case 0x77: // LD (HL), A
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = af.first;
        cycles = 8;
        break;
    case 0x36: // LD (HL), n
        operandDest = this->m_Memory.GetPtrAt(hl.both);
        srcVal = this->m_Memory.ReadMemory8(pc + 1);
        cycles = 12;
        jp_counter = 2;
        break;
    case 0xEA: // LD (nn), A
        operandDest = this->m_Memory.GetPtrAt(this->m_Memory.ReadMemory16(pc + 1));
        srcVal = af.first;
        cycles = 16;
        jp_counter = 3;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    *operandDest = srcVal;
    pc += jp_counter;
}

void Cpu::Execute_Load_16_Val(uint8_t opCode)
{
    uint16_t* operand = nullptr;
    uint8_t cycles = 12;
    switch(opCode)
    {
    case 0x01: // LD BC, nn
        operand = &bc.both;
        break;
    case 0x11: // LD DE, nn
        operand = &de.both;
        break;
    case 0x21: // LD HL, nn
        operand = &hl.both;
        break;
    case 0x31: // LD SP, nn
        operand = &sp;
        break;
    default: 
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    uint16_t val = m_Memory.ReadMemory16(pc + 1);
    *operand = val;

    pc += 3;
}

void Cpu::Execute_Load_HL_A_Dec(uint8_t opCode)
{
    if(opCode != 0x3A && opCode != 0x32)
        throw std::runtime_error("Unimplemented opCode " + opCode);

    m_Memory.SetMemory8(hl.both, af.first);
    Execute_Dec_16(0x2B, true); // DEC HL val
    pc += 1;
}

void Cpu::Execute_Load_HL_A_Inc()
{
    uint8_t cycles = 8;

    m_Memory.SetMemory8(hl.both, af.first);
    Execute_Inc_16(0x23, true); // INC HL val
    pc += 1;
}

void Cpu::Execute_Load_FF00_C_A()
{
    uint8_t cycles = 8;
    m_Memory.SetMemory8(0xFF00 + bc.second, af.first);

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
    
    if (new_val == 0)
        flags.z = true;
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

void Cpu::Execute_Inc_8(uint8_t opCode)
{
    // 4 cycles
    uint8_t* operand = nullptr;
    uint8_t cycles = 4;
    switch(opCode)
    {
    case 0x3C: // INC A
        operand = &af.first;
        break;
    case 0x04: // INC B
        operand = &bc.first;
        break;
    case 0x0C: // INC C
        operand = &bc.second;
        break;
    case 0x14: // INC D
        operand = &de.first;
        break;
    case 0x1C: // INC E
        operand = &de.second;
        break;
    case 0x24: // INC H
        operand = &hl.first;
        break;
    case 0x2C: // INC L
        operand = &hl.second;
        break;
    case 0x34: // INC (HL)
        cycles = 12;
        operand = m_Memory.GetPtrAt(hl.both);
        break;
    default: 
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    flags.h = HalfCarryOnAddition(*operand, 1);
    flags.n = false;

    *operand += 1;
    if (*operand == 0)
        flags.z = true;

    pc += 1;
}

void Cpu::Execute_Inc_16(uint8_t opCode, bool suppress_pc_inc)
{
    // 8 cycles
    uint16_t* operand = nullptr;
    uint8_t cycles = 8;

    switch(opCode)
    {
    case 0x03: // INC BC
        operand = &bc.both;
        break;
    case 0x13: // INC DE
        operand = &de.both;
        break;
    case 0x23: // INC HL
        operand = &hl.both;
        break;
    case 0x33: // INC SP
        operand = &sp;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    *operand += 1;

    if(!suppress_pc_inc)
        pc += 1;
}

void Cpu::Execute_Sub_8(uint8_t opCode)
{
    uint8_t* operand = nullptr;
    uint8_t cycles = 4;
    switch(opCode)
    {
    case 0x97: // SUB A
        operand = &af.first;
        break;
    case 0x90: // SUB B
        operand = &bc.first;
        break;
    case 0x91: // SUB C
        operand = &bc.second;
        break;
    case 0x92: // SUB D
        operand = &de.first;
        break;
    case 0x93: // SUB E
        operand = &de.second;
        break;
    case 0x94: // SUB H
        operand = &hl.first;
        break;
    case 0x95: // SUB L
        operand = &hl.second;
        break;
    case 0x96: // SUB (HL)
        operand = m_Memory.GetPtrAt(hl.both);
        cycles = 8;
        break;
    case 0xD6: // SUB # ???
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    flags.n = true;
    flags.h = !HalfCarryOnSubtraction(af.first, *operand);
    flags.c = !CarryOnSubtraction(af.first, *operand);

    af.first -= *operand;

    if (af.first == 0)
        flags.z = true;

    pc += 1;
}

void Cpu::Execute_SBC_8(uint8_t opCode)
{
    uint8_t* operand = nullptr;
    uint8_t cycles = 4;

    switch(opCode)
    {
    case 0x9F: // SBC A, A
        operand = &af.first;
        break;
    case 0x98: // SBC A, B
        operand = &bc.first;
        break;
    case 0x99: // SBC A, C
        operand = &bc.second;
        break;
    case 0x9A: // SBC A, D
        operand = &de.first;
        break;
    case 0x9B: // SBC A, E
        operand = &de.second;
        break;
    case 0x9C: // SBC A, H
        operand = &hl.first;
        break;
    case 0x9D: // SBC A, L
        operand = &hl.second;
        break;
    case 0x9E: // SBC A, (HL)
        operand = m_Memory.GetPtrAt(hl.both);
        cycles = 8;
        break;
    default:
        throw std::runtime_error("Unknown opCode " + opCode);
    }

    const uint8_t subtractor = *operand + flags.c;

    flags.n = true;
    flags.h = !HalfCarryOnSubtraction(af.first, subtractor);
    flags.c = !CarryOnSubtraction(af.first, subtractor);

    af.first -= subtractor;

    if (af.first == 0)
        flags.z = true;

    pc += 1;
}

void Cpu::Execute_LDH_n_A()
{
    const uint8_t cycles = 12;

    const uint8_t extra_offset = m_Memory.ReadMemory8(pc + 1);
    m_Memory.SetMemory8(0xFF00 + extra_offset, af.first);

    pc += 2;
}

void Cpu::Execute_LDH_A_n()
{
    const uint8_t cycles = 12;

    const uint8_t extra_offset = m_Memory.ReadMemory8(pc + 1);
    af.first = m_Memory.ReadMemory8(0xFF00 + extra_offset);

    pc += 2;
}

void Cpu::Execute_Compare_8(uint8_t opCode)
{
    uint8_t comparator;
    uint8_t cycles = 4;
    uint8_t jp_counter = 1;
    switch(opCode)
    {
    case 0xBF: // CP A
        comparator = af.first;
        break;
    case 0xB8: // CP B
        comparator = bc.first;
        break;
    case 0xB9: // CP C
        comparator = bc.second;
        break;
    case 0xBA: // CP D
        comparator = de.first;
        break;
    case 0xBB: // CP E
        comparator = de.second;
        break;
    case 0xBC: // CP H
        comparator = hl.first;
        break;
    case 0xBD: // CP L
        comparator = hl.second;
        break;
    case 0xBE: // CP (HL)
        comparator = m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0xFE: // CP #
        comparator = m_Memory.ReadMemory8(pc + 1);
        cycles = 8;
        jp_counter = 2;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    if (af.first - comparator == 0)
        flags.z = true;

    flags.n = true;
    if (HalfCarryOnSubtraction(af.first, comparator))
        flags.h = true;
    if (CarryOnSubtraction(af.first, comparator))
        flags.c = true;

    pc += jp_counter;
}

void Cpu::Execute_Or_N(uint8_t opCode)
{
    uint8_t cycles = 4;
    uint8_t* operand = nullptr;
    uint8_t jp_counter = 1;
    switch(opCode)
    {
    case 0xB7: // OR A
        operand = &af.first;
        break;
    case 0xB0: // OR B
        operand = &bc.first;
        break;
    case 0xB1: // OR C
        operand = &bc.second;
        break;
    case 0xB2: // OR D
        operand = &de.first;
        break;
    case 0xB3: // OR E
        operand = &de.second;
        break;
    case 0xB4: // OR H
        operand = &hl.first;
        break;
    case 0xB5: // OR L
        operand = &hl.second;
        break;
    case 0xB6: // OR (HL)
        operand = m_Memory.GetPtrAt(hl.both);
        cycles = 8;
    case 0xF6: // OR (#)
        operand = m_Memory.GetPtrAt(pc + 1);
        cycles = 8;
        jp_counter = 2;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    af.first |= *operand;

    if (af.first == 0)
        flags.z = true;

    flags.n = false;
    flags.h = false;
    flags.c = false;

    pc += jp_counter;
}

void Cpu::Execute_And_N(uint8_t opCode)
{
    uint8_t cycles = 4;
    uint8_t* operand = nullptr;
    uint8_t jp_counter = 1;
    switch(opCode)
    {
    case 0xA7: // AND A
        operand = &af.first;
        break;
    case 0xA0: // AND B
        operand = &bc.first;
        break;
    case 0xA1: // AND C
        operand = &bc.second;
        break;
    case 0xA2: // AND D
        operand = &de.first;
        break;
    case 0xA3: // AND E
        operand = &de.second;
        break;
    case 0xA4: // AND H
        operand = &hl.first;
        break;
    case 0xA5: // AND L
        operand = &hl.second;
        break;
    case 0xA6: // AND (HL)
        operand = m_Memory.GetPtrAt(hl.both);
        cycles = 8;
        break;
    case 0xE6: // AND #
        operand = m_Memory.GetPtrAt(pc + 1);
        cycles = 8;
        jp_counter = 2;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    af.first &= *operand;

    if (af.first == 0)
        flags.z = true;

    flags.n = false;
    flags.h = true;
    flags.c = false;

    pc += jp_counter;
}

uint16_t Cpu::PopStack()
{
    uint16_t val = m_Memory.ReadMemory16(sp);
    sp += 2;
    return val;
}

void Cpu::PushStack(uint16_t val)
{
    sp -= 2;
    m_Memory.SetMemory16(sp, val);
}

void Cpu::Execute_Call()
{
    uint8_t cycles = 12;

    PushStack(pc + 3);
    const uint16_t jp_loc = m_Memory.ReadMemory16(pc + 1);

    pc = jp_loc;
}

void Cpu::Execute_Return(uint8_t opCode)
{
    bool rt = false;
    uint8_t cycles = 8;
    switch(opCode)
    {
    case 0xC9: // RET
        rt = true;
        break;
    case 0xC0: // RET NZ
        rt = !flags.z;
        break;
    case 0xC8: // RET Z
        rt = flags.z;
        break;
    case 0xD0: // RET NC
        rt = !flags.c;
        break;
    case 0xD8: // RET C
        rt = flags.c;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    if(rt)
    {
        const uint16_t rt_address = PopStack();
        pc = rt_address;
    }
}


void Cpu::Execute_Jr_Flag(uint8_t opCode)
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

void Cpu::Execute_Jr_n(uint8_t opCode)
{
    int8_t jump_relative = static_cast<int8_t>(m_Memory.ReadMemory8(pc + 1));
    pc += jump_relative;
}

void Cpu::Execute_Add_HL_Operand(uint8_t opCode)
{
    // 8 cycles

    uint16_t srcVal;
    switch(opCode)
    {
    case 0x09: // ADD HL, BC
        srcVal = bc.both;
        break;
    case 0x19: // ADD HL, DE
        srcVal = de.both;
        break;
    case 0x29: // ADD HL, HL
        srcVal = hl.both;
        break;
    case 0x39: // ADD HL, SP
        srcVal = sp;
        break;
    }

    flags.n = false;
    flags.h = HalfCarryOnAddition(hl.both, srcVal);
    flags.c = CarryOnAddition(hl.both, srcVal);
    hl.both += srcVal;
    pc += 1;
}
