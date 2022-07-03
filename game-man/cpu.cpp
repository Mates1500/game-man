#include "cpu.h"

#include <thread>
#include <stdexcept>

Cpu::Cpu(Memory& memory): m_Memory(memory)
{
    this->sp = SP_INIT_VAL;
    this->rendering_counter_total = 0;
    this->rendering_counter_current_cycle = 0;
    this->current_rendering_state = RenderingState::HBlank;
    this->interrupts_enabled = false;
    this->display_info.currently_render_y = 0;
    this->remaining_di_instructions = 0;
    this->remaining_ei_instructions = 0;
}

void Cpu::StartExecution()
{
    PowerUpSequence();
    this->pc = GB_ROM_ENTRY_POINT;
    
    while(1)
    {
        this->ExecuteInstruction();

        if(remaining_ei_instructions > 0)
        {
            --remaining_ei_instructions;
            if (remaining_ei_instructions == 0)
                EI();
        }
        if(remaining_di_instructions > 0)
        {
            --remaining_di_instructions;
            if (remaining_di_instructions == 0)
                DI();
        }
        const uint8_t interrupt_jp_address = GetInterruptJpAddress();
        if(interrupt_jp_address != 0)
        {
            interrupts_enabled = false;
            m_Memory.SetMemory8(0xFFFF, 0); // disable IME
            PushStack(pc);
            pc = interrupt_jp_address;
        }
    }
}

void Cpu::ExecuteInstruction()
{
    pc_history.insert(pc_history.begin(), pc);
    if(pc_history.size() > 10000)
    {
        pc_history.resize(5000);
    }

    uint8_t op = m_Memory.ReadMemory8(pc);

    switch(op)
    {
    case 0x00: // NOP
        Execute_Nop();
        break;
    case 0x07: // RLCA
        Execute_RLCA();
        break;
    case 0x0F: // RRCA
        Execute_RRCA();
        break;
    case 0x37: // SCF
        Execute_SCF();
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
    case 0x87: // ADD A, A
    case 0x80: // ADD A, B
    case 0x81: // ADD A, C
    case 0x82: // ADD A, D
    case 0x83: // ADD A, E
    case 0x84: // ADD A, H
    case 0x85: // ADD A, L
    case 0x86: // ADD A, (HL)
    case 0xC6: // ADD A, #
        Execute_Add_8(op);
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
        Execute_Cpl();
        break;
    case 0x22: // LDI (HL), A
        // 8 cycles
        Execute_LD_HLI_A();
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
    case 0x0A: // LD A, (BC)
    case 0x1A: // LD A, (DE)
    case 0xFA: // LD A, (nn)
        Execute_Load_8_Operand(op);
        break;
    case 0x3A: // LDD A, (HL)
    case 0x32: // LDD A, (HL)
        // 8 cycles
        Execute_Load_HL_A_Dec(op);
        break;
    case 0x2A: // LDI A, (HL)
        Execute_Load_A_HL_Inc();
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
        Execute_Jp_16();
        break;
    case 0xC2: // JP NZ 16bit
    case 0xCA: // JP Z 16bit
    case 0xD2: // JP NC 16bit
    case 0xDA: // JP C 16bit
        // 12 cycles
        Execute_Jp_16_Flag(op);
        break;
    case 0xE9: // JP HL
        Execute_Jp_HL();
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
        Execute_EI();
        break;
    case 0xF3: // DI
        // 4 cycles
        interrupts_enabled = false;
        m_Memory.SetMemory8(0xFFFF, 0); // disable IME
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
    case 0xF1: // POP AF
    case 0xC1: // POP BC
    case 0xD1: // POP DE
    case 0xE1: // POP HL
        Execute_Pop(op);
        break;
    case 0xF5: // PUSH AF
    case 0xC5: // PUSH BC
    case 0xD5: // PUSH DE
    case 0xE5: // PUSH HL
        Execute_Push(op);
        break;
    case 0xCD: // CALL nn
    case 0xC4: // CALL NZ, nn
    case 0xCC: // CALL Z, nn
    case 0xD4: // CALL NC, nn
    case 0xDC: // CALL C, nn
        Execute_Call(op);
        break;
    case 0xC9: // RET
    case 0xC0: // RET NZ
    case 0xC8: // RET Z
    case 0xD0: // RET NC
    case 0xD8: // RET C
        Execute_Return(op);
        break;
    case 0xD9: // RETI
        Execute_RETI();
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
    case 0xCB: // CB stuff, next switch...
    {
        uint8_t next_opcode = m_Memory.ReadMemory8(pc + 1);
        switch (next_opcode)
        {
        case 0x37: // SWAP A
        case 0x30: // SWAP B
        case 0x31: // SWAP C
        case 0x32: // SWAP D
        case 0x33: // SWAP E
        case 0x34: // SWAP H
        case 0x35: // SWAP L
        case 0x36: // SWAP (HL)
            Execute_Swap(next_opcode);
            break;
        case 0x40: // BIT b, r operations, cba to write them all out
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
            Execute_Bit_Test(next_opcode);
            break;
        case 0x80: // RES b, r operations, cba to write them all out
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            Execute_Reset_Bit(next_opcode);
            break;

        default:
            throw std::runtime_error("Not implemented CB-- op " + next_opcode);
        }
    }
        break;
    case 0xC7: // RST 0x00
    case 0xCF: // RST 0x08
    case 0xD7: // RST 0x10
    case 0xDF: // RST 0x18
    case 0xE7: // RST 0x20
    case 0xEF: // RST 0x28
    case 0xF7: // RST 0x30
    case 0xFF: // RST 0x38
        Execute_Rst(op);
        break;
    default:
        throw std::runtime_error("Not implemented " + op);
    }
}

uint8_t Cpu::GetInterruptJpAddress()
{
    if (!interrupts_enabled)
        return 0;

    uint8_t interrupt_enable = m_Memory.ReadMemory8(0xFFFF);
    if (interrupt_enable == 0)
        return 0;

    uint8_t interrupt_flag = m_Memory.ReadMemory8(0xFF0F);
    if (interrupt_flag == 0)
        return 0;

    if(IsInterruptFlagEnabled(interrupt_enable, InterruptFlags::VBlank))
    {
        if(IsInterruptFlagEnabled(interrupt_flag, InterruptFlags::VBlank))
        {
            return 0x40;
        }
    }


    if (IsInterruptFlagEnabled(interrupt_enable, InterruptFlags::LCDC))
    {
        if (IsInterruptFlagEnabled(interrupt_flag, InterruptFlags::LCDC))
        {
            return 0x48;
        }
    }

    if (IsInterruptFlagEnabled(interrupt_enable, InterruptFlags::TimerOverflow))
    {
        if (IsInterruptFlagEnabled(interrupt_flag, InterruptFlags::TimerOverflow))
        {
            return 0x50;
        }
    }

    if (IsInterruptFlagEnabled(interrupt_enable, InterruptFlags::SerialIOTransferComplete))
    {
        if (IsInterruptFlagEnabled(interrupt_flag, InterruptFlags::SerialIOTransferComplete))
        {
            return 0x58;
        }
    }

    if (IsInterruptFlagEnabled(interrupt_enable, InterruptFlags::TransitionPin))
    {
        if (IsInterruptFlagEnabled(interrupt_flag, InterruptFlags::TransitionPin))
        {
            return 0x60;
        }
    }

    throw std::runtime_error("Cpu::GetInterruptJpAddress failed, enable and flag are non-zero, but no match?");
}

void Cpu::ElapseCycles(uint8_t cycles)
{
    auto* lcdc_status = m_Memory.GetPtrAt(0xFF40);
    const bool display_enabled = (*lcdc_status & 0b10000000) == 0b10000000;

    if (display_enabled)
    {
        CycleRenderingState(cycles);
        CycleRenderingLines(cycles);
    }

    SleepFor(cycles);
}

void Cpu::Execute_Nop()
{
    const uint8_t cycles = 4;

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Cpl()
{
    const uint8_t cycles = 4;
    // one's complement of A register
    // 4 cycles
    af.first = ~af.first;

    flags.n = true;
    flags.h = true;
    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Xor_N(uint8_t opCode)
{
    uint8_t operand;
    uint8_t cycles = 4;
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

    flags.z = af.first == 0;
    flags.n = false;
    flags.h = false;
    flags.c = false;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Load_8_Val(uint8_t opCode)
{
    // 8 cycles
    uint8_t* operand = nullptr;
    uint8_t cycles = 8;
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

    ElapseCycles(cycles);
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
        srcVal = af.first;
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
    case 0x0A: // LD A, (BC)
        operandDest = &af.first;
        srcVal = this->m_Memory.ReadMemory8(bc.both);
        cycles = 8;
        break;
    case 0x1A: // LD A, (DE)
        operandDest = &af.first;
        srcVal = this->m_Memory.ReadMemory8(de.both);
        cycles = 8;
        break;
    case 0xFA: // LD A, (nn)
        operandDest = &af.first;
        srcVal = this->m_Memory.ReadMemory8(this->m_Memory.ReadMemory16(pc + 1));
        cycles = 16;
        jp_counter = 3;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    *operandDest = srcVal;
    pc += jp_counter;

    ElapseCycles(cycles);
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

    ElapseCycles(cycles);
}

void Cpu::Execute_Load_HL_A_Dec(uint8_t opCode)
{
    if(opCode != 0x3A && opCode != 0x32)
        throw std::runtime_error("Unimplemented opCode " + opCode);

    uint8_t cycles = 8;

    m_Memory.SetMemory8(hl.both, af.first);
    Execute_Dec_16(0x2B, true); // DEC HL val
    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Load_A_HL_Inc()
{
    uint8_t cycles = 8;

    af.first = m_Memory.ReadMemory8(hl.both);
    Execute_Inc_16(0x23, true); // INC HL val
    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Load_FF00_C_A()
{
    uint8_t cycles = 8;
    m_Memory.SetMemory8(0xFF00 + bc.second, af.first);

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Dec_8(uint8_t opCode)
{
    uint8_t* operand = nullptr;
    uint8_t cycles = 4;
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
    case 0x35: // DEC (HL)
        cycles = 12;
        operand = m_Memory.GetPtrAt(hl.both);
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    flags.h = (HalfCarryOnSubtraction(*operand, *operand - 1));

    *operand -= 1;

    flags.z = *operand == 0;
    flags.n = true;
    
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Dec_16(uint8_t opCode, bool suppress_pc_inc)
{
    uint16_t* operand = nullptr;
    uint8_t cycles = 8;
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
    {
        pc += 1;

        ElapseCycles(cycles);
    }
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
    flags.z = *operand == 0;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
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

    if (!suppress_pc_inc)
    {
        pc += 1;

        ElapseCycles(cycles);
    }
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

    flags.z = af.first == 0;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
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

    flags.z = af.first == 0;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_LDH_n_A()
{
    const uint8_t cycles = 12;
    const uint8_t extra_offset = m_Memory.ReadMemory8(pc + 1);

    m_Memory.SetMemory8(0xFF00 + extra_offset, af.first);
    pc += 2;

    ElapseCycles(cycles);
}

void Cpu::Execute_LDH_A_n()
{
    const uint8_t cycles = 12;
    const uint8_t extra_offset = m_Memory.ReadMemory8(pc + 1);

    af.first = m_Memory.ReadMemory8(0xFF00 + extra_offset);
    pc += 2;

    ElapseCycles(cycles);
}

void Cpu::Execute_LD_HLI_A()
{
    const uint8_t cycles = 8;

    m_Memory.SetMemory8(hl.both, af.first);
    ++hl.both;
    pc += 1;

    ElapseCycles(cycles);
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

    flags.z = af.first - comparator == 0;

    flags.n = true;
    flags.h = HalfCarryOnSubtraction(af.first, comparator);
    flags.c = CarryOnSubtraction(af.first, comparator);
    UpdateFlagRegister();

    pc += jp_counter;

    ElapseCycles(cycles);
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
        break;
    case 0xF6: // OR (#)
        operand = m_Memory.GetPtrAt(pc + 1);
        cycles = 8;
        jp_counter = 2;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    af.first |= *operand;

    flags.z = af.first == 0;

    flags.n = false;
    flags.h = false;
    flags.c = false;
    UpdateFlagRegister();

    pc += jp_counter;

    ElapseCycles(cycles);
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

    flags.z = af.first == 0;
    flags.n = false;
    flags.h = true;
    flags.c = false;
    UpdateFlagRegister();

    pc += jp_counter;

    ElapseCycles(cycles);
}

void Cpu::Execute_Swap(uint8_t second_opcode)
{
    uint8_t cycles = 8;
    uint8_t* operand = nullptr;
    switch(second_opcode)
    {
    case 0x37: // SWAP A
        operand = &af.first;
        break;
    case 0x30: // SWAP B
        operand = &bc.first;
        break;
    case 0x31: // SWAP C
        operand = &bc.second;
        break;
    case 0x32: // SWAP D
        operand = &de.first;
        break;
    case 0x33: // SWAP E
        operand = &de.second;
        break;
    case 0x34: // SWAP H
        operand = &hl.first;
        break;
    case 0x35: // SWAP L
        operand = &hl.second;
        break;
    case 0x36: // SWAP (HL)
        operand = m_Memory.GetPtrAt(hl.both);
        cycles = 16;
        break;
    default:
        throw std::runtime_error("Unknown SWAP opCode " + second_opcode);
    }

    *operand = Swap(*operand);

    flags.z = *operand == 0;

    flags.n = false;
    flags.h = false;
    flags.c = false;
    UpdateFlagRegister();

    pc += 2;

    ElapseCycles(cycles);
}

void Cpu::Execute_Rst(uint8_t opCode)
{
    uint8_t cycles = 32;
    uint8_t abs_jp;
    switch(opCode)
    {
    case 0xC7: // RST 0x00
        abs_jp = 0x00;
        break;
    case 0xCF: // RST 0x08
        abs_jp = 0x08;
        break;
    case 0xD7: // RST 0x10
        abs_jp = 0x10;
        break;
    case 0xDF: // RST 0x18
        abs_jp = 0x18;
        break;
    case 0xE7: // RST 0x20
        abs_jp = 0x20;
        break;
    case 0xEF: // RST 0x28
        abs_jp = 0x28;
        break;
    case 0xF7: // RST 0x30
        abs_jp = 0x30;
        break;
    case 0xFF: // RST 0x38
        abs_jp = 0x38;
        break;
    default:
        throw std::runtime_error("Unknown opCode " + opCode);
    }

    PushStack(pc + 1);

    pc = abs_jp;

    ElapseCycles(cycles);
}

void Cpu::Execute_Bit_Test(uint8_t second_opcode)
{
    if (second_opcode < 0x40 || second_opcode > 0x7F)
        throw std::runtime_error("Unknown opCode " + second_opcode);

    uint8_t cycles = 8;
    uint8_t* operand = nullptr;
    uint8_t bit_index;

    const uint8_t opcode_base = second_opcode - 0x40;

    if(opcode_base % 8 == 0) // b, B
    {
        operand = &bc.first;
        bit_index = opcode_base / 8;
    }
    else if(opcode_base % 8 == 1) // b, C
    {
        operand = &bc.second;
        bit_index = (opcode_base - 1) / 8;
    }
    else if(opcode_base % 8 == 2) // b, D
    {
        operand = &de.first;
        bit_index = (opcode_base - 2) / 8;
    }
    else if(opcode_base % 8  == 3) // b, E
    {
        operand = &de.second;
        bit_index = (opcode_base - 3) / 8;
    }
    else if (opcode_base % 8 == 4) // b, H
    {
        operand = &hl.first;
        bit_index = (opcode_base - 4) / 8;
    }
    else if (opcode_base % 8 == 5) // b, L
    {
        operand = &hl.second;
        bit_index = (opcode_base - 5) / 8;
    }
    else if (opcode_base % 8 == 6) // b, (HL)
    {
        operand = m_Memory.GetPtrAt(hl.both);
        bit_index = (opcode_base - 6) / 8;
        cycles = 16;
    }
    else if (opcode_base % 8 == 7) // b, A
    {
        operand = &af.first;
        bit_index = (opcode_base - 7) / 8;
    }
    else
    {
        throw std::runtime_error("Couldn't figure out operand or bit number???");
    }

    const uint8_t aligned_bit = 0x1 << bit_index;

    flags.z = (aligned_bit & *operand) == 0;
    flags.n = false;
    flags.h = true;
    UpdateFlagRegister();

    pc += 2;

    ElapseCycles(cycles);
}

void Cpu::Execute_Reset_Bit(uint8_t second_opcode)
{
    if (second_opcode < 0x80 || second_opcode > 0xBF)
        throw std::runtime_error("Unknown opCode " + second_opcode);

    uint8_t cycles = 8;
    uint8_t* operand;
    uint8_t bit_index;

    const uint8_t opcode_base = second_opcode - 0x80;

    if (opcode_base % 8 == 0) // b, B
    {
        operand = &bc.first;
        bit_index = opcode_base / 8;
    }
    else if (opcode_base % 8 == 1) // b, C
    {
        operand = &bc.second;
        bit_index = (opcode_base - 1) / 8;
    }
    else if (opcode_base % 8 == 2) // b, D
    {
        operand = &de.first;
        bit_index = (opcode_base - 2) / 8;
    }
    else if (opcode_base % 8 == 3) // b, E
    {
        operand = &de.second;
        bit_index = (opcode_base - 3) / 8;
    }
    else if (opcode_base % 8 == 4) // b, H
    {
        operand = &hl.first;
        bit_index = (opcode_base - 4) / 8;
    }
    else if (opcode_base % 8 == 5) // b, L
    {
        operand = &hl.second;
        bit_index = (opcode_base - 5) / 8;
    }
    else if (opcode_base % 8 == 6) // b, (HL)
    {
        operand = m_Memory.GetPtrAt(hl.both);
        bit_index = (opcode_base - 6) / 8;
        cycles = 16;
    }
    else if (opcode_base % 8 == 7) // b, A
    {
        operand = &af.first;
        bit_index = (opcode_base - 7) / 8;
    }
    else
    {
        throw std::runtime_error("Couldn't figure out operand or bit number???");
    }

    const uint8_t aligned_bit_inverted = ~(0x1 << bit_index);
    *operand = *operand & aligned_bit_inverted;

    pc += 2;

    ElapseCycles(cycles);
}

void Cpu::Execute_RRCA()
{
    uint8_t cycles = 4;
    const bool bit_zero = af.first & 0x1;
    af.first = std::rotr(af.first, 1);

    flags.z = af.first == 0;
    flags.n = false;
    flags.h = false;
    flags.c = bit_zero;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_RLCA()
{
    uint8_t cycles = 4;
    const bool bit_seven = af.first & 0x80;
    af.first = std::rotl(af.first, 1);

    flags.z = af.first == 0;
    flags.n = false;
    flags.h = false;
    flags.c = bit_seven;
    UpdateFlagRegister();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_SCF()
{
    uint8_t cycles = 4;

    flags.n = false;
    flags.h = false;
    flags.c = true;
    UpdateFlagRegister();

    sp += 1;
}

void Cpu::Execute_EI()
{
    const uint8_t cycles = 4;
    // 4 cycles
    remaining_ei_instructions = 2; // this instruction will be subtracted too
    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_DI()
{
    const uint8_t cycles = 4;
    // 4 cycles
    remaining_di_instructions = 2; // this instruction will be subtracted too
    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::EI()
{
    interrupts_enabled = true;
}

void Cpu::DI()
{
    interrupts_enabled = false;
    //m_Memory.SetMemory8(0xFFFF, 0); // disable IME
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

void Cpu::Execute_Call(uint8_t opCode)
{
    uint8_t cycles = 12;
    bool jp;

    switch(opCode)
    {
    case 0xCD: // CALL nn
        jp = true;
        break;
    case 0xC4: // CALL NZ, nn
        jp = !flags.z;
        break;
    case 0xCC: // CALL Z, nn
        jp = flags.z;
        break;
    case 0xD4: // CALL NC, nn
        jp = !flags.c;
        break;
    case 0xDC: // CALL C, nn
        jp = flags.c;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    if (jp)
    {
        PushStack(pc + 3);
        const uint16_t jp_loc = m_Memory.ReadMemory16(pc + 1);

        pc = jp_loc;
    }
    else
    {
        pc += 3;
    }

    ElapseCycles(cycles);
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
    else
    {
        pc += 1;
    }

    ElapseCycles(cycles);
}

void Cpu::Execute_RETI()
{
    const uint8_t cycles = 8;

    const uint16_t rt_address = PopStack();
    pc = rt_address;

    interrupts_enabled = true;

    ElapseCycles(cycles);
}

void Cpu::Execute_Pop(uint8_t opCode)
{
    uint8_t cycles = 12;
    uint16_t* dest = nullptr;
    switch(opCode)
    {
    case 0xF1: // POP AF
        dest = &af.both;
        break;
    case 0xC1: // POP BC
        dest = &bc.both;
        break;
    case 0xD1: // POP DE
        dest = &de.both;
        break;
    case 0xE1: // POP HL
        dest = &hl.both;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    *dest = PopStack();

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::Execute_Push(uint8_t opCode)
{
    uint8_t cycles = 16;
    uint16_t val;
    switch(opCode)
    {
    case 0xF5: // PUSH AF
        val = af.both;
        break;
    case 0xC5: // PUSH BC
        val = bc.both;
        break;
    case 0xD5: // PUSH DE
        val = de.both;
        break;
    case 0xE5: // PUSH HL
        val = hl.both;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    PushStack(val);

    pc += 1;

    ElapseCycles(cycles);
}

void Cpu::SleepFor(uint8_t cycles)
{
    auto now = std::chrono::steady_clock::now();
    auto local_offset = std::chrono::duration<double>(0);

    local_offset = now - last_tick;

    double adjusted_sleep_s = static_cast<double>(cycles) / GB_CLOCK;

    if (local_offset.count() < adjusted_sleep_s)
    {
        adjusted_sleep_s -= local_offset.count();

        const auto adjusted_sleep_ns = static_cast<int>(adjusted_sleep_s * 1000000);

        std::this_thread::sleep_for(std::chrono::nanoseconds(adjusted_sleep_ns));
    }

    last_tick = std::chrono::steady_clock::now();
}

void Cpu::UpdateFlagRegister()
{
    // layout 76543210
    //        ZNHC0000
    af.second = (flags.z << 7) | (flags.n << 6) | (flags.h << 5) | (flags.c << 4);
}

void Cpu::PowerUpSequence()
{
    af.first = 0x01;
    flags.z = true;
    flags.n = false;
    flags.h = true;
    flags.c = true;
    UpdateFlagRegister(); // f default = 0xB0 = 0b10110000
    bc.both = 0x13;
    de.both = 0xD8;
    hl.both = 0x14D;
    sp = 0xFFFE;

    m_Memory.SetMemory8(0xFF05, 0); // TIMA
    m_Memory.SetMemory8(0xFF06, 0); // TMA
    m_Memory.SetMemory8(0xFF07, 0); // TAC
    m_Memory.SetMemory8(0xFF10, 0x80); // NR10
    m_Memory.SetMemory8(0xFF11, 0xBF); // NR11
    m_Memory.SetMemory8(0xFF12, 0xF3); // NR12
    m_Memory.SetMemory8(0xFF14, 0xBF); // NR14
    m_Memory.SetMemory8(0xFF16, 0x3F); // NR21
    m_Memory.SetMemory8(0xFF17, 0); // NR22
    m_Memory.SetMemory8(0xFF19, 0xBF); // NR24
    m_Memory.SetMemory8(0xFF1A, 0x7F); // NR30
    m_Memory.SetMemory8(0xFF1B, 0xFF); // NR31
    m_Memory.SetMemory8(0xFF1C, 0x9F); // NR32
    m_Memory.SetMemory8(0xFF1E, 0xBF); // NR33
    m_Memory.SetMemory8(0xFF20, 0xFF); // NR41
    m_Memory.SetMemory8(0xFF21, 0); // NR42
    m_Memory.SetMemory8(0xFF22, 0); // NR43
    m_Memory.SetMemory8(0xFF23, 0xBF); // NR30
    m_Memory.SetMemory8(0xFF24, 0x77); // NR50
    m_Memory.SetMemory8(0xFF25, 0xF3); // NR51
    m_Memory.SetMemory8(0xFF26, 0xF1); // NR52
    m_Memory.SetMemory8(0xFF40, 0x91); // LCDC
    m_Memory.SetMemory8(0xFF42, 0); // SCY
    m_Memory.SetMemory8(0xFF43, 0); // SCX
    m_Memory.SetMemory8(0xFF45, 0); // LYC
    m_Memory.SetMemory8(0xFF47, 0xFC); // BGP
    m_Memory.SetMemory8(0xFF48, 0xFF); // 0BP0
    m_Memory.SetMemory8(0xFF49, 0xFF); // 0BP1
    m_Memory.SetMemory8(0xFF4A, 0); // WY
    m_Memory.SetMemory8(0xFF4B, 0); // WX
    m_Memory.SetMemory8(0xFFFF, 0); // IE
}

void Cpu::CycleRenderingState(uint8_t cycles)
{
    bool changed = false;
    rendering_counter_total += cycles;
    rendering_counter_current_cycle += cycles;

    if (rendering_counter_current_cycle < OAM_USED_CYCLES) // lowest number of cycles to trigger anything
        return;

    switch(current_rendering_state)
    {
    case RenderingState::HBlank:
        if(rendering_counter_current_cycle >= HBLANK_CYCLES && rendering_counter_total < VBLANK_START_CYCLE)
        {
            rendering_counter_current_cycle -= HBLANK_CYCLES;
            current_rendering_state = RenderingState::OAM_Used;
            changed = true;
        }
        else if(rendering_counter_current_cycle >= HBLANK_CYCLES && rendering_counter_total >= VBLANK_START_CYCLE)
        {
            rendering_counter_current_cycle -= HBLANK_CYCLES;
            current_rendering_state = RenderingState::VBlank;
            changed = true;
        }
        break;
    case RenderingState::VBlank: 
        if(rendering_counter_current_cycle >= VBLANK_CYCLES && rendering_counter_total >= VBLANK_END_CYCLE)
        {
            rendering_counter_total -= VBLANK_END_CYCLE;
            rendering_counter_current_cycle -= VBLANK_CYCLES;
            current_rendering_state = RenderingState::HBlank;
            changed = true;
        }
        break;
    case RenderingState::OAM_Used:
        if(rendering_counter_current_cycle >= OAM_USED_CYCLES)
        {
            rendering_counter_current_cycle -= OAM_USED_CYCLES;
            current_rendering_state = RenderingState::OAM_RAM_Used;
            changed = true;
        }
        break;
    case RenderingState::OAM_RAM_Used: 
        if(rendering_counter_current_cycle >= OAM_RAM_USED_CYCLES)
        {
            rendering_counter_current_cycle -= OAM_RAM_USED_CYCLES;
            current_rendering_state = RenderingState::HBlank;
            changed = true;
        }
        break;
    }

    if(changed)
    {
        auto* lcdc_stat = m_Memory.GetPtrAt(0xFF41);
        *lcdc_stat |= static_cast<uint8_t>(current_rendering_state);
    }
}

void Cpu::CycleRenderingLines(uint8_t cycles)
{
    display_info.rendering_current_cycle += cycles;

    if (display_info.rendering_current_cycle < 456)
        return;

    display_info.rendering_current_cycle -= 456;
    if(display_info.currently_render_y < 153)
    {
        display_info.currently_render_y += 1;
    }
    else
    {
        display_info.currently_render_y = 0;
    }

    m_Memory.SetMemory8(0xFF44, display_info.currently_render_y); // set LY

    const uint8_t lyc_compare = m_Memory.ReadMemory8(0xFF45);
    uint8_t stat = m_Memory.ReadMemory8(0xFF41);
    if(display_info.currently_render_y == lyc_compare)
    {
        stat = stat | 0b100; // set coincidence flag
        m_Memory.SetMemory8(0xFF41, stat);
    }
    else
    {
        stat = stat & 0b11111011; // reset coincidence flag, keep everything else
        m_Memory.SetMemory8(0xFF41, stat);
    }
}


void Cpu::Execute_Jr_Flag(uint8_t opCode)
{
    uint8_t cycles = 8;
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
        ElapseCycles(cycles);
        return;
    }
    uint8_t orig_val = m_Memory.ReadMemory8(pc + 1);
    int8_t jump_relative = static_cast<int8_t>(orig_val);
    pc += 2 + jump_relative;

    ElapseCycles(cycles);
}

void Cpu::Execute_Jr_n(uint8_t opCode)
{
    uint8_t cycles = 8;

    int8_t jump_relative = static_cast<int8_t>(m_Memory.ReadMemory8(pc + 1)) + 2;
    pc += jump_relative;

    ElapseCycles(cycles);
}

void Cpu::Execute_Jp_HL()
{
    uint8_t cycles = 4;

    pc = hl.both;

    ElapseCycles(cycles);
}

void Cpu::Execute_Jp_16()
{
    const uint8_t cycles = 12;
    // 12 cycles
    pc = m_Memory.ReadMemory16(pc + 1);

    ElapseCycles(cycles);
}

void Cpu::Execute_Jp_16_Flag(uint8_t opCode)
{
    const uint8_t cycles = 12;
    bool jp = false;
    switch(opCode)
    {
    case 0xC2: // JP NZ nn
        jp = !flags.z;
        break;
    case 0xCA: // JP Z nn
        jp = flags.z;
        break;
    case 0xD2: // JP NC nn
        jp = !flags.c;
        break;
    case 0xDA: // JP C nn
        jp = flags.c;
        break;
    default:
        throw std::runtime_error("Unknown opCode " + opCode);
    }

    if (jp)
        pc = m_Memory.ReadMemory16(pc + 1);
    else
        pc += 3;

    ElapseCycles(cycles);
}

void Cpu::Execute_Add_HL_Operand(uint8_t opCode)
{
    uint8_t cycles = 8;

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
    default:
        throw std::runtime_error("Unknown opCode " + opCode);
    }

    flags.n = false;
    flags.h = HalfCarryOnAddition(hl.both, srcVal);
    flags.c = CarryOnAddition(hl.both, srcVal);
    UpdateFlagRegister();
    hl.both += srcVal;
    pc += 1;

    ElapseCycles(8);
}

void Cpu::Execute_Add_8(uint8_t opCode)
{
    uint8_t srcVal;
    uint8_t cycles = 4;
    uint8_t jp_counter = 1;
    switch(opCode)
    {
    case 0x87: // ADD A, A
        srcVal = af.first;
        break;
    case 0x80: // ADD A, B
        srcVal = bc.first;
        break;
    case 0x81: // ADD A, C
        srcVal = bc.second;
        break;
    case 0x82: // ADD A, D
        srcVal = de.first;
        break;
    case 0x83: // ADD A, E
        srcVal = de.second;
        break;
    case 0x84: // ADD A, H
        srcVal = hl.first;
        break;
    case 0x85: // ADD A, L
        srcVal = hl.second;
        break;
    case 0x86: // ADD A, (HL)
        srcVal = m_Memory.ReadMemory8(hl.both);
        cycles = 8;
        break;
    case 0xC6: // ADD A, #
        srcVal = m_Memory.ReadMemory8(pc + 1);
        cycles = 8;
        jp_counter = 2;
        break;
    default:
        throw std::runtime_error("Unimplemented opCode " + opCode);
    }

    flags.n = false;
    flags.c = CarryOnAddition(af.first, srcVal);
    flags.h = HalfCarryOnAddition(af.first, srcVal);


    af.first += srcVal;

    flags.z = af.first == 0;
    UpdateFlagRegister();

    pc += jp_counter;

    ElapseCycles(cycles);
}
