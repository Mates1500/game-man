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

    this->m_memoryBuffer.at(offset) = (val & 0x00FF); // considering we're on LE, low byte first
    this->m_memoryBuffer.at(offset + 1) = (val >> 8); // high second
    // TODO: Big Endian?
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
    return *reinterpret_cast<uint16_t*>(&m_memoryBuffer[offset]); // if we consider we're on Little Endian, TODO: BE?
}

uint8_t* Memory::GetPtrAt(uint16_t offset)
{
    if (offset > this->m_memoryBuffer.size())
        throw std::runtime_error("Memory::GetPtrAt - Invalid location, out of bounds " + offset);

    return &this->m_memoryBuffer.at(offset);
}
