#include "OutputFormatter.hpp"
#include "services/MetadataService.hpp"
#include "utils/StringUtils.hpp"
#include "utils/TimeUtils.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace filemgr {

OutputFormatter::OutputFormatter()
    : colorEnabled_(::isatty(STDOUT_FILENO) != 0) {}

// ── Colorized text ──────────────────────────────────────────────────────────

std::string OutputFormatter::colorizeEntry(const std::string& name,
                                            bool isDir,
                                            bool isSymlink,
                                            bool isExecutable) const {
    if (!colorEnabled_) return name;

    if (isSymlink)     return colorize(name, Color::BOLD_CYAN);
    if (isDir)         return colorize(name + "/", Color::BOLD_BLUE);
    if (isExecutable)  return colorize(name, Color::BOLD_GREEN);
    return name;
}

std::string OutputFormatter::colorizePermissions(const std::string& perms) const {
    if (!colorEnabled_) return perms;

    std::string result;
    for (char c : perms) {
        if (c == '-') {
            result += colorize(std::string_view(&c, 1), Color::DIM);
        } else if (c == 'r') {
            result += colorize(std::string_view(&c, 1), Color::YELLOW);
        } else if (c == 'w') {
            result += colorize(std::string_view(&c, 1), Color::RED);
        } else if (c == 'x' || c == 's' || c == 't') {
            result += colorize(std::string_view(&c, 1), Color::GREEN);
        } else {
            result += c;
        }
    }
    return result;
}

std::string OutputFormatter::colorize(std::string_view text,
                                       const char* color) const {
    if (!colorEnabled_) return std::string(text);
    return std::string(color) + std::string(text) + Color::RESET;
}

// ── Formatted output ────────────────────────────────────────────────────────

std::string OutputFormatter::formatLongEntry(const std::string& path) const {
    auto meta = MetadataService::getMetadata(path);

    std::string typeChar;
    switch (meta.type) {
        case FileType::DIRECTORY:    typeChar = "d"; break;
        case FileType::SYMLINK:      typeChar = "l"; break;
        case FileType::BLOCK_DEVICE: typeChar = "b"; break;
        case FileType::CHAR_DEVICE:  typeChar = "c"; break;
        case FileType::FIFO:         typeChar = "p"; break;
        case FileType::SOCKET:       typeChar = "s"; break;
        default:                     typeChar = "-"; break;
    }

    std::string perms = StringUtils::formatPermissions(meta.permissions);
    std::string colorPerms = typeChar + colorizePermissions(perms);

    std::ostringstream oss;
    oss << colorPerms
        << "  " << std::setw(2) << meta.hardLinks
        << " " << StringUtils::padRight(meta.ownerName, 8)
        << " " << StringUtils::padRight(meta.groupName, 8)
        << " " << StringUtils::padLeft(
                     StringUtils::humanReadableSize(meta.size), 8)
        << "  " << TimeUtils::formatTimeLs(meta.modificationTime)
        << "  ";

    // Colorize name
    std::string name = std::filesystem::path(path).filename().string();
    bool isExec = (meta.permissions & S_IXUSR) != 0;
    oss << colorizeEntry(name,
                         meta.type == FileType::DIRECTORY,
                         meta.type == FileType::SYMLINK,
                         isExec);

    return oss.str();
}

std::string OutputFormatter::formatSimpleListing(
    const std::vector<DirectoryEntry>& entries) const
{
    std::ostringstream oss;

    for (const auto& entry : entries) {
        bool isExec = false;
        // Check execute permission via stat
        struct stat st{};
        if (::stat(entry.path.c_str(), &st) == 0) {
            isExec = (st.st_mode & S_IXUSR) != 0;
        }

        oss << colorizeEntry(entry.name, entry.isDirectory,
                             entry.isSymlink, isExec)
            << "\n";
    }

    return oss.str();
}

std::string OutputFormatter::formatLongListing(
    const std::vector<DirectoryEntry>& entries) const
{
    std::ostringstream oss;

    for (const auto& entry : entries) {
        try {
            oss << formatLongEntry(entry.path) << "\n";
        } catch (const std::exception& e) {
            oss << formatError(std::string("Cannot stat: ") + entry.path) << "\n";
        }
    }

    return oss.str();
}

std::string OutputFormatter::formatError(std::string_view message) const {
    return colorize("error: ", Color::BOLD_RED) + std::string(message);
}

std::string OutputFormatter::formatSuccess(std::string_view message) const {
    return colorize("✓ ", Color::BOLD_GREEN) + std::string(message);
}

std::string OutputFormatter::formatWarning(std::string_view message) const {
    return colorize("warning: ", Color::YELLOW) + std::string(message);
}

} // namespace filemgr
