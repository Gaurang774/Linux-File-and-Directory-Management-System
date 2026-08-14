#include "DirectoryService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"
#include "utils/TimeUtils.hpp"

#include <algorithm>
#include <filesystem>
#include <regex>
#include <sstream>
#include <sys/stat.h>

namespace filemgr {

namespace fs = std::filesystem;

// ── Create directory ────────────────────────────────────────────────────────

void DirectoryService::createDirectory(const std::string& path, bool recursive) {
    PathUtils::validatePath(path);

    if (PathUtils::exists(path)) {
        if (PathUtils::isDirectory(path)) {
            LOG_INFO("Directory already exists: " + path);
            return;
        }
        throw PathExistsException(path);
    }

    std::error_code ec;
    bool created = recursive
        ? fs::create_directories(fs::path(path), ec)
        : fs::create_directory(fs::path(path), ec);

    if (ec) {
        if (ec.value() == EACCES || ec.value() == EPERM) {
            throw PermissionDeniedException(path);
        }
        throw IOErrorException("Failed to create directory", ec.value(), path);
    }
    if (!created && !recursive) {
        // Parent didn't exist and recursive wasn't requested
        throw IOErrorException("Parent directory does not exist", ENOENT, path);
    }
    LOG_INFO("Created directory: " + path);
}

// ── Remove empty directory ──────────────────────────────────────────────────

void DirectoryService::removeDirectory(const std::string& path) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isDirectory(path)) {
        throw InvalidPathException(path, "Not a directory");
    }

    std::error_code ec;
    if (!fs::remove(fs::path(path), ec)) {
        if (ec.value() == ENOTEMPTY) {
            throw DirectoryNotEmptyException(path);
        }
        throw IOErrorException("Failed to remove directory", ec.value(), path);
    }
    LOG_INFO("Removed directory: " + path);
}

// ── Remove directory recursively ────────────────────────────────────────────

size_t DirectoryService::removeDirectoryRecursive(const std::string& path,
                                                   bool dryRun) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isDirectory(path)) {
        throw InvalidPathException(path, "Not a directory");
    }

    if (dryRun) {
        // Count items that would be removed
        size_t count = 0;
        auto entries = DirectoryIterator::traverse(path, {});
        for (const auto& e : entries) {
            LOG_INFO("[DRY RUN] Would remove: " + e.path);
            ++count;
        }
        LOG_INFO("[DRY RUN] Would remove directory: " + path);
        return count + 1;
    }

    std::error_code ec;
    auto count = fs::remove_all(fs::path(path), ec);
    if (ec) {
        throw IOErrorException("Failed to remove directory recursively",
                               ec.value(), path);
    }
    LOG_INFO("Removed " + std::to_string(count) + " items from: " + path);
    return static_cast<size_t>(count);
}

// ── List directory ──────────────────────────────────────────────────────────

std::vector<DirectoryEntry> DirectoryService::listDirectory(
    const std::string& path,
    const ListOptions& options)
{
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isDirectory(path)) {
        throw InvalidPathException(path, "Not a directory");
    }

    TraversalOptions travOpts;
    travOpts.maxDepth = options.recursive ? options.maxDepth : 0;
    travOpts.includeHidden = options.showHidden;

    // Build filter
    std::regex nameRegex;
    bool hasNameFilter = false;
    if (!options.filterPattern.empty()) {
        std::string regexStr = StringUtils::globToRegex(options.filterPattern);
        nameRegex = std::regex(regexStr, std::regex::ECMAScript);
        hasNameFilter = true;
    }

    travOpts.filter = [&](const DirectoryEntry& entry) -> bool {
        // Extension filter
        if (!options.filterExtension.empty()) {
            std::string ext = fs::path(entry.name).extension().string();
            if (ext != options.filterExtension) return false;
        }
        // Name pattern filter
        if (hasNameFilter) {
            if (!std::regex_match(entry.name, nameRegex)) return false;
        }
        return true;
    };

    // Sort strategy
    switch (options.sortBy) {
        case ListOptions::SortBy::NAME:
            travOpts.sorter = [](const DirectoryEntry& a, const DirectoryEntry& b) {
                // Directories first, then by name
                if (a.isDirectory != b.isDirectory) return a.isDirectory;
                return a.name < b.name;
            };
            break;
        case ListOptions::SortBy::SIZE:
            travOpts.sorter = [](const DirectoryEntry& a, const DirectoryEntry& b) {
                return a.size > b.size;  // largest first
            };
            break;
        case ListOptions::SortBy::DATE:
            travOpts.sorter = [](const DirectoryEntry& a, const DirectoryEntry& b) {
                return a.modTime > b.modTime;  // newest first
            };
            break;
        case ListOptions::SortBy::NONE:
            break;
    }

    return DirectoryIterator::traverse(path, travOpts);
}

// ── Copy directory recursively ──────────────────────────────────────────────

size_t DirectoryService::copyDirectory(const std::string& src,
                                        const std::string& dst,
                                        bool dryRun) {
    PathUtils::validatePath(src);
    PathUtils::validatePath(dst);

    if (!PathUtils::exists(src)) {
        throw FileNotFoundException(src);
    }
    if (!PathUtils::isDirectory(src)) {
        throw InvalidPathException(src, "Source is not a directory");
    }

    if (dryRun) {
        auto entries = DirectoryIterator::traverse(src, {});
        for (const auto& e : entries) {
            LOG_INFO("[DRY RUN] Would copy: " + e.path);
        }
        return entries.size();
    }

    std::error_code ec;
    fs::copy(fs::path(src), fs::path(dst),
             fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    if (ec) {
        throw IOErrorException("Failed to copy directory", ec.value(), src);
    }

    // Count items copied
    auto entries = DirectoryIterator::traverse(dst, {});
    LOG_INFO("Copied " + std::to_string(entries.size()) + " items: " +
             src + " → " + dst);
    return entries.size();
}

// ── Tree view ───────────────────────────────────────────────────────────────

std::string DirectoryService::treeView(const std::string& path,
                                        int maxDepth,
                                        bool showLong) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }

    std::ostringstream oss;
    oss << fs::path(path).filename().string() << "\n";

    TraversalOptions opts;
    opts.maxDepth = maxDepth;
    opts.sorter = [](const DirectoryEntry& a, const DirectoryEntry& b) {
        if (a.isDirectory != b.isDirectory) return a.isDirectory;
        return a.name < b.name;
    };

    auto entries = DirectoryIterator::traverse(path, opts);

    // Build tree with box-drawing characters
    // We need to track which entries are last at each depth level
    // For simplicity, we process entries and compute prefixes
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];

        // Determine if this is the last entry at its depth among siblings
        bool isLast = true;
        for (size_t j = i + 1; j < entries.size(); ++j) {
            if (entries[j].depth < entry.depth) break;
            if (entries[j].depth == entry.depth) {
                isLast = false;
                break;
            }
        }

        // Build prefix
        std::string prefix;
        for (int d = 0; d < entry.depth; ++d) {

            // Check if ancestor at this level has more siblings
            bool ancestorHasMore = false;
            for (size_t j = i + 1; j < entries.size(); ++j) {
                if (entries[j].depth < d) break;
                if (entries[j].depth == d) {
                    ancestorHasMore = true;
                    break;
                }
            }
            prefix += ancestorHasMore ? "│   " : "    ";
        }

        // Connector
        std::string connector = isLast ? "└── " : "├── ";

        // Entry name with optional long format
        std::string display = entry.name;
        if (entry.isDirectory) display += "/";

        if (showLong) {
            std::string sizeStr = entry.isRegularFile
                ? StringUtils::humanReadableSize(entry.size)
                : "-";
            std::string timeStr = TimeUtils::formatTimeLs(entry.modTime);
            display += "  [" + sizeStr + ", " + timeStr + "]";
        }

        oss << prefix << connector << display << "\n";
    }

    return oss.str();
}

} // namespace filemgr
