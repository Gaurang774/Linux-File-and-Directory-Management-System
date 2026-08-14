#include "StringUtils.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

namespace filemgr {
namespace StringUtils {

// ── Human-readable file sizes ───────────────────────────────────────────────

std::string humanReadableSize(uint64_t bytes) {
    static const std::array<const char*, 5> units = {"B", "KB", "MB", "GB", "TB"};

    if (bytes == 0) return "0 B";

    auto value = static_cast<double>(bytes);
    size_t unitIdx = 0;

    while (value >= 1024.0 && unitIdx < units.size() - 1) {
        value /= 1024.0;
        ++unitIdx;
    }

    std::ostringstream oss;
    if (unitIdx == 0) {
        oss << bytes << " " << units[unitIdx];
    } else {
        oss << std::fixed << std::setprecision(1) << value << " " << units[unitIdx];
    }
    return oss.str();
}

// ── Glob-to-regex conversion ────────────────────────────────────────────────

std::string globToRegex(std::string_view pattern) {
    std::string regex;
    regex.reserve(pattern.size() * 2);
    regex += '^';

    for (size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        switch (c) {
            case '*':  regex += ".*";    break;
            case '?':  regex += '.';     break;
            case '.':  regex += "\\.";   break;
            case '\\': regex += "\\\\";  break;
            case '(':  regex += "\\(";   break;
            case ')':  regex += "\\)";   break;
            case '{':  regex += "\\{";   break;
            case '}':  regex += "\\}";   break;
            case '[':  regex += '[';     break;
            case ']':  regex += ']';     break;
            case '+':  regex += "\\+";   break;
            case '^':  regex += "\\^";   break;
            case '$':  regex += "\\$";   break;
            case '|':  regex += "\\|";   break;
            default:   regex += c;       break;
        }
    }

    regex += '$';
    return regex;
}

// ── Permission formatting ───────────────────────────────────────────────────

std::string formatPermissions(mode_t mode) {
    std::string perms(9, '-');

    // Owner
    if (mode & S_IRUSR) perms[0] = 'r';
    if (mode & S_IWUSR) perms[1] = 'w';
    if (mode & S_IXUSR) perms[2] = 'x';

    // Group
    if (mode & S_IRGRP) perms[3] = 'r';
    if (mode & S_IWGRP) perms[4] = 'w';
    if (mode & S_IXGRP) perms[5] = 'x';

    // Other
    if (mode & S_IROTH) perms[6] = 'r';
    if (mode & S_IWOTH) perms[7] = 'w';
    if (mode & S_IXOTH) perms[8] = 'x';

    // Special bits (setuid, setgid, sticky)
    if (mode & S_ISUID) perms[2] = (perms[2] == 'x') ? 's' : 'S';
    if (mode & S_ISGID) perms[5] = (perms[5] == 'x') ? 's' : 'S';
    if (mode & S_ISVTX) perms[8] = (perms[8] == 'x') ? 't' : 'T';

    return perms;
}

std::string formatOctalPermissions(mode_t mode) {
    std::ostringstream oss;
    oss << std::oct << std::setw(4) << std::setfill('0')
        << (mode & 07777);  // mask to permission bits only
    return oss.str();
}

// ── Common string operations ────────────────────────────────────────────────

std::string toLower(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string trim(std::string_view str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    auto end = str.find_last_not_of(" \t\n\r");
    return std::string(str.substr(start, end - start + 1));
}

std::vector<std::string> split(std::string_view str, char delim) {
    std::vector<std::string> result;
    size_t start = 0;

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == delim) {
            if (i > start) {
                result.emplace_back(str.substr(start, i - start));
            }
            start = i + 1;
        }
    }
    if (start < str.size()) {
        result.emplace_back(str.substr(start));
    }
    return result;
}

bool startsWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() &&
           str.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() &&
           str.substr(str.size() - suffix.size()) == suffix;
}

std::string padLeft(std::string_view str, size_t width, char fill) {
    if (str.size() >= width) return std::string(str);
    return std::string(width - str.size(), fill) + std::string(str);
}

std::string padRight(std::string_view str, size_t width, char fill) {
    if (str.size() >= width) return std::string(str);
    return std::string(str) + std::string(width - str.size(), fill);
}

} // namespace StringUtils
} // namespace filemgr
