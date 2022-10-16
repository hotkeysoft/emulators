#pragma once

#include <cstdio>
#include <vector>
#include <filesystem>

namespace hscommon::fileUtil
{
    using SelectFileFilter = std::pair<std::string, std::string>;
    using SelectFileFilters = std::vector<SelectFileFilter>;

    bool SelectFile(std::filesystem::path& path, SelectFileFilters filter = {}, bool addAllFilesFilter = true);

    // Thin file wrapper that automatically closes the file
    // when going out of scope.
    // WARNING: Don't use this if you need to keep the file handle
    // alive out this object's scope
    // WARNING: Don't call fclose() on the handle, use Close()
    class File
    {
    public:
        File() : m_handle(nullptr) {}

        File(const char* path, const char* mode)
        {
            Open(path, mode);
        }
        ~File()
        {
            if (m_handle)
            {
                fclose(m_handle);
            }
        }

        File(const File&) = delete;
        File& operator=(const File&) = delete;
        File(File&&) = delete;
        File& operator=(File&&) = delete;

        void Attach(File& other)
        {
            Close();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }

        bool Open(const char* path, const char* mode)
        {
            Close();
            m_handle = fopen(path, mode);
            return m_handle != nullptr;
        }
        void Close()
        {
            if (m_handle)
            {
                fclose(m_handle);
                m_handle = nullptr;
            }
        }

        operator FILE*() const { return m_handle; }

    protected:
        FILE* m_handle = nullptr;
    };
}
