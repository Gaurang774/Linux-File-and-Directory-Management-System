#pragma once

///////////////////////////////////////////////////////////////////////////////
// PathUtils.hpp — Path manipulation utilities
//
// Wraps std::filesystem::path with convenience functions and validation.
///////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <string>
#include <string_view>

namespace filemgr {
namespace PathUtils {

namespace fs = std::filesystem;

/// Normalize path: resolve ".", "..", collapse redundant separators.
/// Uses weakly_canonical (does not require path to exist).
std::string normalize(std::string_view path);

/// Join base and relative path segments
std::string join(std::string_view base, std::string_view relative);

/// Get absolute path
std::string absolutePath(std::string_view path);

/// Extract filename from path (e.g., "/a/b/c.txt" → "c.txt")
std::string filename(std::string_view path);

/// Extract file extension (e.g., "c.txt" → ".txt")
std::string extension(std::string_view path);

/// Extract stem (filename without extension, e.g., "c.txt" → "c")
std::string stem(std::string_view path);

/// Get parent directory (e.g., "/a/b/c.txt" → "/a/b")
std::string parentDir(std::string_view path);

/// Check if path exists
bool exists(std::string_view path);

/// Check if path is a regular file
bool isFile(std::string_view path);

/// Check if path is a directory
bool isDirectory(std::string_view path);

/// Check if path is a symbolic link
bool isSymlink(std::string_view path);

/// Get current working directory
std::string currentDir();

/// Validate a user-supplied path string (not empty, reasonable length)
/// Throws InvalidPathException on failure.
void validatePath(std::string_view path);

} // namespace PathUtils
} // namespace filemgr
