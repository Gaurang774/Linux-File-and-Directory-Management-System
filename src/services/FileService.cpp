#include "FileService.hpp"
#include "core/FileHandle.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>

namespace filemgr {

namespace fs = std::filesystem;

void FileService::createFile(const std::string& path) {
    PathUtils::validatePath(path);

    if (PathUtils::exists(path)) {
        // Touch: update modification time (like POSIX touch)
        std::error_code ec;
        fs::last_write_time(fs::path(path), fs::file_time_type::clock::now(), ec);
        if (ec) {
            throw IOErrorException("Failed to update modification time", ec.value(), path);
        }
        LOG_INFO("Touched existing file: " + path);
        return;
    }

    // Create new empty file
    auto handle = FileHandle::create(path, 0644);
    handle.close();
    LOG_INFO("Created file: " + path);
}

std::string FileService::readFile(const std::string& path) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isFile(path)) {
        throw InvalidPathException(path, "Not a regular file");
    }

    auto handle = FileHandle::open(path, O_RDONLY);
    std::string content = handle.readAll();
    LOG_DEBUG("Read " + std::to_string(content.size()) + " bytes from: " + path);
    return content;
}

void FileService::writeFile(const std::string& path, std::string_view content) {
    PathUtils::validatePath(path);

    auto handle = FileHandle::create(path, 0644);
    handle.write(content);
    LOG_INFO("Wrote " + std::to_string(content.size()) + " bytes to: " + path);
}

void FileService::appendFile(const std::string& path, std::string_view content) {
    PathUtils::validatePath(path);

    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        int err = errno;
        if (err == EACCES || err == EPERM) {
            throw PermissionDeniedException(path);
        }
        throw IOErrorException("Failed to open for append", err, path);
    }

    // Wrap in FileHandle for RAII
    // We need to manually construct; use a small scope
    {
        const char* ptr = content.data();
        size_t remaining = content.size();
        while (remaining > 0) {
            ssize_t written = ::write(fd, ptr, remaining);
            if (written < 0) {
                int err = errno;
                ::close(fd);
                throw IOErrorException("Append write failed", err, path);
            }
            auto w = static_cast<size_t>(written);
            ptr += w;
            remaining -= w;
        }
    }
    ::close(fd);
    LOG_INFO("Appended " + std::to_string(content.size()) + " bytes to: " + path);
}

void FileService::deleteFile(const std::string& path) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isFile(path) && !PathUtils::isSymlink(path)) {
        throw InvalidPathException(path, "Not a regular file (use rmdir for directories)");
    }

    std::error_code ec;
    if (!fs::remove(fs::path(path), ec)) {
        if (ec) {
            throw IOErrorException("Failed to delete file", ec.value(), path);
        }
    }
    LOG_INFO("Deleted file: " + path);
}

void FileService::copyFile(const std::string& src, const std::string& dst,
                            bool overwrite) {
    PathUtils::validatePath(src);
    PathUtils::validatePath(dst);

    if (!PathUtils::exists(src)) {
        throw FileNotFoundException(src);
    }
    if (!PathUtils::isFile(src)) {
        throw InvalidPathException(src, "Source is not a regular file");
    }
    if (!overwrite && PathUtils::exists(dst)) {
        throw PathExistsException(dst);
    }

    std::error_code ec;
    auto options = fs::copy_options::overwrite_existing;
    fs::copy_file(fs::path(src), fs::path(dst),
                  overwrite ? options : fs::copy_options::none, ec);
    if (ec) {
        throw IOErrorException("Failed to copy file", ec.value(), src);
    }
    LOG_INFO("Copied: " + src + " → " + dst);
}

void FileService::moveFile(const std::string& src, const std::string& dst) {
    PathUtils::validatePath(src);
    PathUtils::validatePath(dst);

    if (!PathUtils::exists(src)) {
        throw FileNotFoundException(src);
    }

    std::error_code ec;
    fs::rename(fs::path(src), fs::path(dst), ec);
    if (ec) {
        // rename failed (possibly cross-device), fall back to copy + delete
        if (PathUtils::isFile(src)) {
            copyFile(src, dst, true);
            deleteFile(src);
        } else {
            throw IOErrorException("Failed to move", ec.value(), src);
        }
    }
    LOG_INFO("Moved: " + src + " → " + dst);
}

bool FileService::exists(const std::string& path) {
    return PathUtils::exists(path);
}

} // namespace filemgr
