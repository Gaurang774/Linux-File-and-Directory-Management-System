#pragma once

///////////////////////////////////////////////////////////////////////////////
// Exceptions.hpp — Custom exception hierarchy for file-mgr
//
// Base:  FileMgrException
//   ├── FileNotFoundException
//   ├── PermissionDeniedException
//   ├── DirectoryNotEmptyException
//   ├── PathExistsException
//   ├── InvalidPathException
//   ├── InvalidArgumentException
//   └── IOErrorException
//
// Every exception carries:
//   • Human-readable message
//   • System errno (0 if not applicable)
//   • Source path (empty if not applicable)
///////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace filemgr {

// ── Base exception ──────────────────────────────────────────────────────────

class FileMgrException : public std::runtime_error {
public:
    FileMgrException(const std::string& message,
                     int errnoCode = 0,
                     const std::string& path = "")
        : std::runtime_error(buildMessage(message, errnoCode, path))
        , errnoCode_(errnoCode)
        , path_(path)
    {}

    [[nodiscard]] int errnoCode()       const noexcept { return errnoCode_; }
    [[nodiscard]] const std::string& path() const noexcept { return path_; }

private:
    int         errnoCode_;
    std::string path_;

    static std::string buildMessage(const std::string& msg,
                                    int errnoCode,
                                    const std::string& path) {
        std::string result = msg;
        if (!path.empty()) {
            result += ": '" + path + "'";
        }
        if (errnoCode != 0) {
            result += " (";
            result += std::strerror(errnoCode);
            result += ")";
        }
        return result;
    }
};

// ── Derived exceptions ──────────────────────────────────────────────────────

class FileNotFoundException : public FileMgrException {
public:
    explicit FileNotFoundException(const std::string& path)
        : FileMgrException("File not found", ENOENT, path) {}
};

class PermissionDeniedException : public FileMgrException {
public:
    explicit PermissionDeniedException(const std::string& path)
        : FileMgrException("Permission denied", EACCES, path) {}
};

class DirectoryNotEmptyException : public FileMgrException {
public:
    explicit DirectoryNotEmptyException(const std::string& path)
        : FileMgrException("Directory not empty", ENOTEMPTY, path) {}
};

class PathExistsException : public FileMgrException {
public:
    explicit PathExistsException(const std::string& path)
        : FileMgrException("Path already exists", EEXIST, path) {}
};

class InvalidPathException : public FileMgrException {
public:
    explicit InvalidPathException(const std::string& path,
                                  const std::string& reason = "Invalid path")
        : FileMgrException(reason, 0, path) {}
};

class InvalidArgumentException : public FileMgrException {
public:
    explicit InvalidArgumentException(const std::string& message)
        : FileMgrException(message) {}
};

class IOErrorException : public FileMgrException {
public:
    IOErrorException(const std::string& message,
                     int errnoCode,
                     const std::string& path = "")
        : FileMgrException(message, errnoCode, path) {}
};

} // namespace filemgr
