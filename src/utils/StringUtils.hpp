#pragma once

///////////////////////////////////////////////////////////////////////////////
// StringUtils.hpp — String helper functions
//
// • Human-readable file sizes
// • Glob-to-regex conversion
// • Permission formatting (mode_t → rwx string)
// • Common string operations
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

namespace filemgr {
namespace StringUtils {

/// Convert byte count to human-readable string: "1.2 GB", "345 B"
std::string humanReadableSize(uint64_t bytes);

/// Convert a glob pattern ("*.cpp") to a POSIX-extended regex string
std::string globToRegex(std::string_view pattern);

/// Format file permission bits as "rwxr-xr-x"
std::string formatPermissions(mode_t mode);

/// Format file permission bits as octal "0755"
std::string formatOctalPermissions(mode_t mode);

/// Convert string to lowercase
std::string toLower(std::string_view str);

/// Trim leading and trailing whitespace
std::string trim(std::string_view str);

/// Split string by delimiter
std::vector<std::string> split(std::string_view str, char delim);

/// Check prefix
bool startsWith(std::string_view str, std::string_view prefix);

/// Check suffix
bool endsWith(std::string_view str, std::string_view suffix);

/// Left-pad string to given width
std::string padLeft(std::string_view str, size_t width, char fill = ' ');

/// Right-pad string to given width
std::string padRight(std::string_view str, size_t width, char fill = ' ');

} // namespace StringUtils
} // namespace filemgr
