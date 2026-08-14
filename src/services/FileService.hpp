#pragma once

///////////////////////////////////////////////////////////////////////////////
// FileService.hpp — File operations (create, read, write, delete, copy, move)
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <string_view>

namespace filemgr {

class FileService {
public:
    /// Create an empty file (touch equivalent). No-op if file exists.
    static void createFile(const std::string& path);

    /// Read entire file contents as string.
    static std::string readFile(const std::string& path);

    /// Write content to file (truncate existing or create new).
    static void writeFile(const std::string& path, std::string_view content);

    /// Append content to file (create if not exists).
    static void appendFile(const std::string& path, std::string_view content);

    /// Delete a file. Throws if not found or permission denied.
    static void deleteFile(const std::string& path);

    /// Copy a file from src to dst.
    /// If overwrite is false and dst exists, throws PathExistsException.
    static void copyFile(const std::string& src, const std::string& dst,
                         bool overwrite = false);

    /// Move (rename) a file from src to dst.
    static void moveFile(const std::string& src, const std::string& dst);

    /// Check if file exists.
    static bool exists(const std::string& path);
};

} // namespace filemgr
