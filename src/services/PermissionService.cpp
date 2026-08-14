#include "PermissionService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"

#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

namespace filemgr {

PermissionInfo PermissionService::getPermissions(const std::string& path) {
    PathUtils::validatePath(path);

    struct stat st{};
    if (::lstat(path.c_str(), &st) < 0) {
        int err = errno;
        if (err == ENOENT) throw FileNotFoundException(path);
        if (err == EACCES) throw PermissionDeniedException(path);
        throw IOErrorException("stat failed", err, path);
    }

    PermissionInfo info;
    info.mode        = st.st_mode;
    info.octalString = StringUtils::formatOctalPermissions(st.st_mode);
    info.rwxString   = StringUtils::formatPermissions(st.st_mode);

    if      (S_ISDIR(st.st_mode))  info.typeChar = "d";
    else if (S_ISLNK(st.st_mode))  info.typeChar = "l";
    else if (S_ISBLK(st.st_mode))  info.typeChar = "b";
    else if (S_ISCHR(st.st_mode))  info.typeChar = "c";
    else if (S_ISFIFO(st.st_mode)) info.typeChar = "p";
    else if (S_ISSOCK(st.st_mode)) info.typeChar = "s";
    else                           info.typeChar = "-";

    return info;
}

void PermissionService::setPermissions(const std::string& path, mode_t mode) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }

    if (::chmod(path.c_str(), mode) < 0) {
        int err = errno;
        if (err == EACCES || err == EPERM) {
            throw PermissionDeniedException(path);
        }
        throw IOErrorException("chmod failed", err, path);
    }

    LOG_INFO("Changed permissions of " + path + " to " +
             StringUtils::formatOctalPermissions(mode));
}

mode_t PermissionService::parseMode(const std::string& modeStr) {
    if (modeStr.empty()) {
        throw InvalidArgumentException("Mode string is empty");
    }

    // Try octal first: "755", "0755"
    bool allOctal = true;
    for (char c : modeStr) {
        if (c < '0' || c > '7') {
            allOctal = false;
            break;
        }
    }

    if (allOctal && modeStr.size() >= 3 && modeStr.size() <= 4) {
        try {
            unsigned long val = std::stoul(modeStr, nullptr, 8);
            return static_cast<mode_t>(val);
        } catch (...) {
            throw InvalidArgumentException("Invalid octal mode: " + modeStr);
        }
    }

    // Try rwx format: "rwxr-xr-x"
    if (modeStr.size() == 9) {
        mode_t mode = 0;
        // Owner
        if (modeStr[0] == 'r') mode |= S_IRUSR;
        if (modeStr[1] == 'w') mode |= S_IWUSR;
        if (modeStr[2] == 'x') mode |= S_IXUSR;
        else if (modeStr[2] == 's') mode |= S_IXUSR | S_ISUID;
        else if (modeStr[2] == 'S') mode |= S_ISUID;
        // Group
        if (modeStr[3] == 'r') mode |= S_IRGRP;
        if (modeStr[4] == 'w') mode |= S_IWGRP;
        if (modeStr[5] == 'x') mode |= S_IXGRP;
        else if (modeStr[5] == 's') mode |= S_IXGRP | S_ISGID;
        else if (modeStr[5] == 'S') mode |= S_ISGID;
        // Other
        if (modeStr[6] == 'r') mode |= S_IROTH;
        if (modeStr[7] == 'w') mode |= S_IWOTH;
        if (modeStr[8] == 'x') mode |= S_IXOTH;
        else if (modeStr[8] == 't') mode |= S_IXOTH | S_ISVTX;
        else if (modeStr[8] == 'T') mode |= S_ISVTX;
        return mode;
    }

    throw InvalidArgumentException(
        "Invalid mode format: '" + modeStr + "'. "
        "Use octal (e.g., 755) or symbolic (e.g., u+x)");
}

mode_t PermissionService::parseSymbolicMode(const std::string& modeStr,
                                             mode_t currentMode) {
    // Parse symbolic modes like: u+x, go-w, a+rx, u=rwx
    mode_t result = currentMode & 07777;  // strip file type bits

    size_t i = 0;
    while (i < modeStr.size()) {
        // Parse who: u, g, o, a
        mode_t whoMask = 0;
        while (i < modeStr.size() && modeStr[i] != '+' &&
               modeStr[i] != '-' && modeStr[i] != '=') {
            switch (modeStr[i]) {
                case 'u': whoMask |= S_IRWXU; break;
                case 'g': whoMask |= S_IRWXG; break;
                case 'o': whoMask |= S_IRWXO; break;
                case 'a': whoMask |= S_IRWXU | S_IRWXG | S_IRWXO; break;
                default:
                    throw InvalidArgumentException(
                        "Invalid symbolic mode character: " +
                        std::string(1, modeStr[i]));
            }
            ++i;
        }
        if (whoMask == 0) whoMask = S_IRWXU | S_IRWXG | S_IRWXO;  // default: all

        if (i >= modeStr.size()) {
            throw InvalidArgumentException("Expected +, -, or = in mode: " + modeStr);
        }

        char op = modeStr[i++];

        // Parse what: r, w, x
        mode_t permBits = 0;
        while (i < modeStr.size() && modeStr[i] != ',' &&
               modeStr[i] != '+' && modeStr[i] != '-' && modeStr[i] != '=') {
            mode_t bit = 0;
            switch (modeStr[i]) {
                case 'r': bit = S_IRUSR | S_IRGRP | S_IROTH; break;
                case 'w': bit = S_IWUSR | S_IWGRP | S_IWOTH; break;
                case 'x': bit = S_IXUSR | S_IXGRP | S_IXOTH; break;
                default:
                    throw InvalidArgumentException(
                        "Invalid permission character: " +
                        std::string(1, modeStr[i]));
            }
            permBits |= (bit & whoMask);
            ++i;
        }

        // Apply operation
        switch (op) {
            case '+': result |= permBits; break;
            case '-': result &= ~permBits; break;
            case '=': result = (result & ~whoMask) | permBits; break;
            default: break;
        }

        // Skip comma separator
        if (i < modeStr.size() && modeStr[i] == ',') ++i;
    }

    return result;
}

void PermissionService::changeOwner(const std::string& path,
                                     uid_t uid, gid_t gid) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }

    if (::chown(path.c_str(), uid, gid) < 0) {
        int err = errno;
        if (err == EACCES || err == EPERM) {
            throw PermissionDeniedException(path);
        }
        throw IOErrorException("chown failed", err, path);
    }

    LOG_INFO("Changed owner of " + path + " to " +
             std::to_string(uid) + ":" + std::to_string(gid));
}

} // namespace filemgr
