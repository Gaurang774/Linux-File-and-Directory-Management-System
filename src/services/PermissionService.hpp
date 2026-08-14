#pragma once

///////////////////////////////////////////////////////////////////////////////
// PermissionService.hpp — File permission management
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sys/types.h>

namespace filemgr {

/// Permission information
struct PermissionInfo {
    mode_t      mode;          // Raw mode bits
    std::string octalString;   // "0755"
    std::string rwxString;     // "rwxr-xr-x"
    std::string typeChar;      // "d", "-", "l", etc.
};

class PermissionService {
public:
    /// Get current permissions.
    static PermissionInfo getPermissions(const std::string& path);

    /// Set permissions (mode can be octal like 0755 or symbolic).
    static void setPermissions(const std::string& path, mode_t mode);

    /// Parse a permission string to mode_t.
    /// Accepts: "755", "0755", "rwxr-xr-x", "u+x", "go-w"
    /// Throws InvalidArgumentException on bad input.
    static mode_t parseMode(const std::string& modeStr);

    /// Parse a permission string relative to current mode.
    /// For symbolic modes like "u+x", "go-w".
    static mode_t parseSymbolicMode(const std::string& modeStr, mode_t currentMode);

    /// Change file owner (requires root/CAP_CHOWN for other users).
    static void changeOwner(const std::string& path, uid_t uid, gid_t gid);
};

} // namespace filemgr
