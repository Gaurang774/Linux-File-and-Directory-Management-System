#pragma once

///////////////////////////////////////////////////////////////////////////////
// FileHandle.hpp — RAII wrapper around POSIX file descriptors
//
// Move-only. Auto-closes fd on destruction.
// Provides read/write/seek via low-level POSIX I/O.
///////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <fcntl.h>
#include <sys/types.h>

namespace filemgr {

class FileHandle {
public:
    // ── Construction / Destruction ──────────────────────────────────────
    FileHandle() noexcept : fd_(-1) {}
    ~FileHandle();

    // Move-only
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // ── Factory methods ─────────────────────────────────────────────────

    /// Open existing file with given flags (O_RDONLY, O_WRONLY, O_RDWR, etc.)
    /// Throws FileNotFoundException or PermissionDeniedException on failure.
    static FileHandle open(const std::string& path, int flags);

    /// Create new file (or truncate existing) with given permission mode.
    /// Throws IOErrorException on failure.
    static FileHandle create(const std::string& path, mode_t mode = 0644);

    // ── I/O operations ──────────────────────────────────────────────────

    /// Read up to `count` bytes. Returns data read (may be shorter at EOF).
    std::string read(size_t count) const;

    /// Read entire file contents from current position to EOF.
    std::string readAll() const;

    /// Write data to file. Returns bytes written.
    size_t write(std::string_view data) const;

    /// Seek to offset. Returns new position.
    off_t seek(off_t offset, int whence = SEEK_SET) const;

    /// Get file size via fstat.
    uint64_t size() const;

    // ── State queries ───────────────────────────────────────────────────

    [[nodiscard]] bool isValid() const noexcept { return fd_ >= 0; }
    [[nodiscard]] int fd() const noexcept { return fd_; }

    /// Close the file descriptor explicitly.
    void close();

private:
    int fd_;

    explicit FileHandle(int fd) noexcept : fd_(fd) {}
};

} // namespace filemgr
