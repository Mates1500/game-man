#include "file_handle.h"

FileHandle::FileHandle(std::string const& path): m_Handle(new std::ifstream(path, std::ios::binary | std::ios::ate))
{
}

std::vector<uint8_t>& FileHandle::GetFileContentsVector()
{
    if (!this->m_fileContents.empty())
        return this->m_fileContents;


    if (this->m_Handle == nullptr)
        throw std::runtime_error("FileHandle::m_Handle is null");

    auto end = this->m_Handle->tellg();
    this->m_Handle->seekg(0, std::ios::beg);

    auto size = static_cast<std::size_t>(end - this->m_Handle->tellg());
    if (size > 0x8000)
        throw std::runtime_error("FileHandle::m_Handle size is over 0x8000");

    this->m_fileContents.resize(size);

    if (size == 0)
        throw std::runtime_error("FileHandle::m_Handle size is 0");

    if (!this->m_Handle->read(reinterpret_cast<char*>(this->m_fileContents.data()), this->m_fileContents.size()))
        throw std::runtime_error(std::strerror(errno));

    return this->m_fileContents;
}
