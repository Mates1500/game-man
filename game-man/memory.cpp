#include "memory.h"

Memory::Memory() : m_memoryBuffer(GB_MEMORY_BUFFER_SIZE), m_memoryMap(reinterpret_cast<MemoryMap*>(m_memoryBuffer.data()))
{

}

void Memory::SetMemory8(uint16_t offset, uint8_t val)
{
    this->m_memoryBuffer.at(offset) = val;
}

void Memory::SetMemory16(uint16_t offset, uint16_t val)
{
    // TODO: check length constraint
    this->m_memoryBuffer.at(offset) = (val & 0x00FF); // LE, therefore low first
    this->m_memoryBuffer.at(offset + 1) = (val >> 8); // high second
}

uint8_t Memory::ReadMemory8(uint16_t offset)
{
    return this->m_memoryBuffer.at(offset);
}

uint16_t Memory::ReadMemory16(uint16_t offset)
{
    return *reinterpret_cast<uint16_t*>(&m_memoryBuffer[offset]);
}
