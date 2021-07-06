#pragma once
#include <fstream>
#include <vector>


class FileHandle
{
public:
    FileHandle(std::string const& path);

    /* copies all contents into a vector, if you're manipulating
     a multigigabyte file here, good luck */
    std::vector<uint8_t>& GetFileContentsVector();
private:
    struct SafeFileHandleDeleter
    {
        void operator()(std::ifstream* stream) const
        {
            if (stream != nullptr)
                stream->close();
        }
    };
    using SafeFileHandle = std::unique_ptr<std::ifstream, SafeFileHandleDeleter>;

    SafeFileHandle m_Handle;
    std::vector<uint8_t> m_fileContents;
};
