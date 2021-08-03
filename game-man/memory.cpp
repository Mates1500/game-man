#include "memory.h"

#include <stdexcept>

Memory::Memory() : m_memoryBuffer(GB_MEMORY_BUFFER_SIZE + 1), m_memoryMap(reinterpret_cast<MemoryMap*>(m_memoryBuffer.data()))
{

}

void Memory::SetMemory8(uint16_t offset, uint8_t val)
{
    this->m_memoryBuffer.at(offset) = val;
}

void Memory::SetMemory16(uint16_t offset, uint16_t val)
{
    if (offset + 2 > this->m_memoryBuffer.size())
        throw std::runtime_error("Memory::SetMemory16 - offset + 2 bytes > memoryBuffer");

    this->m_memoryBuffer.at(offset + 1) = (val & 0x00FF); // LE to BE, therefore high first
    this->m_memoryBuffer.at(offset) = (val >> 8); // low second
}

void Memory::SetRomMemory(std::vector<uint8_t>& rom_contents) const
{
    if (rom_contents.size() != (sizeof(MemoryMap::rom) + sizeof(MemoryMap::switchable_rom_bank)))
        throw std::runtime_error("Only vectors of size 0x8000 are allowed");

    for (int i = 0; i < rom_contents.size() / 2; ++i)
    {
        this->m_memoryMap->rom[i] = rom_contents.at(i);
    }

    for (int i = 0; i < rom_contents.size() / 2; ++i)
    {
        this->m_memoryMap->switchable_rom_bank[i] = rom_contents.at(rom_contents.size() / 2 + i);
    }
    
}

uint8_t Memory::ReadMemory8(uint16_t offset)
{
    return this->m_memoryBuffer.at(offset);
}

uint16_t Memory::ReadMemory16(uint16_t offset)
{
    uint16_t result = *reinterpret_cast<uint16_t*>(&m_memoryBuffer[offset]); // wrong interpretation because memory is Big Endian mapped, but CPU is most likely Little Endian
    result =  (result >> 8) | (result << 8); // BE to LE
    return result;
}

uint8_t* Memory::GetPtrAt(uint16_t offset)
{
    if (offset > this->m_memoryBuffer.size())
        throw std::runtime_error("Memory::GetPtrAt - Invalid location, out of bounds " + offset);

    return &this->m_memoryBuffer.at(offset);
}
