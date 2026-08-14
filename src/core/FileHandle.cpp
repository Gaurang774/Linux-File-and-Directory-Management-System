#include "FileHandle.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>

namespace filemgr {

// ── Destructor ──────────────────────────────────────────────────────────────

FileHandle::~FileHandle() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

// ── Move operations ─────────────────────────────────────────────────────────

FileHandle::FileHandle(FileHandle&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

// ── Factory methods ─────────────────────────────────────────────────────────

FileHandle FileHandle::open(const std::string& path, int flags) {
    int fd = ::open(path.c_str(), flags);
    if (fd < 0) {
        int err = errno;
        if (err == ENOENT) {
            throw FileNotFoundException(path);
        } else if (err == EACCES || err == EPERM) {
            throw PermissionDeniedException(path);
        } else {
            throw IOErrorException("Failed to open file", err, path);
        }
    }
    LOG_DEBUG("Opened file: " + path + " (fd=" + std::to_string(fd) + ")");
    return FileHandle(fd);
}

FileHandle FileHandle::create(const std::string& path, mode_t mode) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        int err = errno;
        if (err == EACCES || err == EPERM) {
            throw PermissionDeniedException(path);
        } else {
            throw IOErrorException("Failed to create file", err, path);
        }
    }
    LOG_DEBUG("Created file: " + path + " (fd=" + std::to_string(fd) + ")");
    return FileHandle(fd);
}

// ── I/O operations ──────────────────────────────────────────────────────────

std::string FileHandle::read(size_t count) const {
    if (!isValid()) {
        throw IOErrorException("Read on invalid file handle", EBADF);
    }

    std::string buffer(count, '\0');
    ssize_t bytesRead = ::read(fd_, buffer.data(), count);

    if (bytesRead < 0) {
        throw IOErrorException("Read failed", errno);
    }

    buffer.resize(static_cast<size_t>(bytesRead));
    return buffer;
}

std::string FileHandle::readAll() const {
    if (!isValid()) {
        throw IOErrorException("Read on invalid file handle", EBADF);
    }

    // Get current position and file size
    off_t current = ::lseek(fd_, 0, SEEK_CUR);
    off_t end = ::lseek(fd_, 0, SEEK_END);
    ::lseek(fd_, current, SEEK_SET);  // restore position

    if (current < 0 || end < 0) {
        // Not seekable (pipe, etc.) — read in chunks
        std::string result;
        constexpr size_t chunkSize = 8192;
        char chunk[chunkSize];
        ssize_t bytesRead;

        while ((bytesRead = ::read(fd_, chunk, chunkSize)) > 0) {
            result.append(chunk, static_cast<size_t>(bytesRead));
        }
        if (bytesRead < 0) {
            throw IOErrorException("Read failed", errno);
        }
        return result;
    }

    auto remaining = static_cast<size_t>(end - current);
    return read(remaining);
}

size_t FileHandle::write(std::string_view data) const {
    if (!isValid()) {
        throw IOErrorException("Write on invalid file handle", EBADF);
    }

    const char* ptr = data.data();
    size_t remaining = data.size();
    size_t totalWritten = 0;

    while (remaining > 0) {
        ssize_t written = ::write(fd_, ptr, remaining);
        if (written < 0) {
            if (errno == EINTR) continue;  // interrupted, retry
            throw IOErrorException("Write failed", errno);
        }
        auto w = static_cast<size_t>(written);
        ptr += w;
        remaining -= w;
        totalWritten += w;
    }

    return totalWritten;
}

off_t FileHandle::seek(off_t offset, int whence) const {
    if (!isValid()) {
        throw IOErrorException("Seek on invalid file handle", EBADF);
    }

    off_t result = ::lseek(fd_, offset, whence);
    if (result < 0) {
        throw IOErrorException("Seek failed", errno);
    }
    return result;
}

uint64_t FileHandle::size() const {
    if (!isValid()) {
        throw IOErrorException("Size query on invalid file handle", EBADF);
    }

    struct stat st{};
    if (::fstat(fd_, &st) < 0) {
        throw IOErrorException("fstat failed", errno);
    }
    return static_cast<uint64_t>(st.st_size);
}

void FileHandle::close() {
    if (fd_ >= 0) {
        if (::close(fd_) < 0) {
            LOG_WARN("close() failed for fd " + std::to_string(fd_));
        }
        fd_ = -1;
    }
}

} // namespace filemgr
