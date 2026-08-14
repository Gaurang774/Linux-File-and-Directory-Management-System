#include "PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <climits>  // PATH_MAX

namespace filemgr {
namespace PathUtils {

namespace fs = std::filesystem;

std::string normalize(std::string_view path) {
    validatePath(path);
    std::error_code ec;
    auto result = fs::weakly_canonical(fs::path(path), ec);
    if (ec) {
        // Fall back to lexically normal if weakly_canonical fails
        return fs::path(path).lexically_normal().string();
    }
    return result.string();
}

std::string join(std::string_view base, std::string_view relative) {
    return (fs::path(base) / fs::path(relative)).string();
}

std::string absolutePath(std::string_view path) {
    validatePath(path);
    std::error_code ec;
    auto result = fs::absolute(fs::path(path), ec);
    if (ec) {
        throw InvalidPathException(std::string(path),
                                   "Cannot resolve absolute path");
    }
    return result.string();
}

std::string filename(std::string_view path) {
    return fs::path(path).filename().string();
}

std::string extension(std::string_view path) {
    return fs::path(path).extension().string();
}

std::string stem(std::string_view path) {
    return fs::path(path).stem().string();
}

std::string parentDir(std::string_view path) {
    return fs::path(path).parent_path().string();
}

bool exists(std::string_view path) {
    std::error_code ec;
    return fs::exists(fs::path(path), ec);
}

bool isFile(std::string_view path) {
    std::error_code ec;
    return fs::is_regular_file(fs::path(path), ec);
}

bool isDirectory(std::string_view path) {
    std::error_code ec;
    return fs::is_directory(fs::path(path), ec);
}

bool isSymlink(std::string_view path) {
    std::error_code ec;
    return fs::is_symlink(fs::path(path), ec);
}

std::string currentDir() {
    std::error_code ec;
    auto result = fs::current_path(ec);
    if (ec) {
        throw IOErrorException("Failed to get current directory", ec.value());
    }
    return result.string();
}

void validatePath(std::string_view path) {
    if (path.empty()) {
        throw InvalidPathException("", "Path cannot be empty");
    }
    if (path.size() > PATH_MAX) {
        throw InvalidPathException(std::string(path.substr(0, 64)) + "...",
                                   "Path exceeds maximum length");
    }
    // Check for null bytes
    if (path.find('\0') != std::string_view::npos) {
        throw InvalidPathException(std::string(path.substr(0, 64)),
                                   "Path contains null byte");
    }
    LOG_DEBUG("Validated path: " + std::string(path));
}

} // namespace PathUtils
} // namespace filemgr
