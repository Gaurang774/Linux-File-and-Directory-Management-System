#pragma once

///////////////////////////////////////////////////////////////////////////////
// OutputFormatter.hpp — Pretty printing with ANSI colors and alignment
///////////////////////////////////////////////////////////////////////////////

#include "core/DirectoryIterator.hpp"
#include "services/MetadataService.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace filemgr {

/// ANSI color codes
namespace Color {
    inline const char* RESET      = "\033[0m";
    inline const char* BOLD       = "\033[1m";
    inline const char* DIM        = "\033[2m";

    inline const char* RED        = "\033[31m";
    inline const char* GREEN      = "\033[32m";
    inline const char* YELLOW     = "\033[33m";
    inline const char* BLUE       = "\033[34m";
    inline const char* MAGENTA    = "\033[35m";
    inline const char* CYAN       = "\033[36m";
    inline const char* WHITE      = "\033[37m";

    inline const char* BOLD_RED   = "\033[1;31m";
    inline const char* BOLD_GREEN = "\033[1;32m";
    inline const char* BOLD_BLUE  = "\033[1;34m";
    inline const char* BOLD_CYAN  = "\033[1;36m";
}

class OutputFormatter {
public:
    /// Initialize formatter. Auto-detects terminal color support.
    OutputFormatter();

    /// Disable/enable color output
    void setColorEnabled(bool enabled) { colorEnabled_ = enabled; }
    [[nodiscard]] bool isColorEnabled() const { return colorEnabled_; }

    // ── Colorized text ──────────────────────────────────────────────────

    /// Colorize a filename based on its type
    std::string colorizeEntry(const std::string& name, bool isDir,
                              bool isSymlink, bool isExecutable) const;

    /// Colorize permission string (green for enabled, dim for -)
    std::string colorizePermissions(const std::string& perms) const;

    /// Apply a color to text
    std::string colorize(std::string_view text, const char* color) const;

    // ── Formatted output ────────────────────────────────────────────────

    /// Format a directory listing entry in long format
    /// Example: "drwxr-xr-x  user  group  4.0 KB  Mar 15 14:30  dirname/"
    std::string formatLongEntry(const std::string& path) const;

    /// Format a simple listing (names only, columns)
    std::string formatSimpleListing(
        const std::vector<DirectoryEntry>& entries) const;

    /// Format a long listing (all metadata)
    std::string formatLongListing(
        const std::vector<DirectoryEntry>& entries) const;

    /// Format an error message
    std::string formatError(std::string_view message) const;

    /// Format a success message
    std::string formatSuccess(std::string_view message) const;

    /// Format a warning message
    std::string formatWarning(std::string_view message) const;

private:
    bool colorEnabled_;
};

} // namespace filemgr
