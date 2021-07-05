#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#define GB_MEMORY_BUFFER_SIZE 0xFFFF

struct MemoryMap
{
    uint8_t rom[0x4000];
    uint8_t switchable_rom_bank[0x4000];
    uint8_t video_ram[0x2000];
    uint8_t switchable_ram_bank[0x2000];
    uint8_t internal_ram[0x2000];
    uint8_t reserved1[0x1E00];
    uint8_t sprite_attributes[0x00A0];
    uint8_t reserved2[0x0060];
    uint8_t i_o[0x004B];
    uint8_t reserved3[0x0033];
    uint8_t high_ram[0x0080];
    uint8_t interrupt_register;
};

class Memory
{
public:
    Memory();
    void SetMemory8(uint16_t offset, uint8_t val);
    void SetMemory16(uint16_t offset, uint16_t val);
    uint8_t ReadMemory8(uint16_t offset);
    uint16_t ReadMemory16(uint16_t offset);
private:
    std::vector<uint8_t> m_memoryBuffer;
    MemoryMap* m_memoryMap;
};
