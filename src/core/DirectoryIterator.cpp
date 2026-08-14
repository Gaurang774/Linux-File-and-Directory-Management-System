#include "DirectoryIterator.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <algorithm>
#include <sys/stat.h>

namespace filemgr {

namespace fs = std::filesystem;

// ── Public API ──────────────────────────────────────────────────────────────

std::vector<DirectoryEntry> DirectoryIterator::traverse(
    const std::string& rootPath,
    const TraversalOptions& options)
{
    fs::path root(rootPath);
    std::error_code ec;

    if (!fs::exists(root, ec)) {
        throw FileNotFoundException(rootPath);
    }
    if (!fs::is_directory(root, ec)) {
        throw InvalidPathException(rootPath, "Not a directory");
    }

    std::vector<DirectoryEntry> results;
    std::vector<uint64_t> visitedInodes;

    // Track the root inode for cycle detection
    struct stat rootStat{};
    if (::stat(rootPath.c_str(), &rootStat) == 0) {
        visitedInodes.push_back(static_cast<uint64_t>(rootStat.st_ino));
    }

    traverseRecursive(root, 0, options, results, visitedInodes);

    return results;
}

// ── Internal recursive DFS ──────────────────────────────────────────────────

void DirectoryIterator::traverseRecursive(
    const fs::path& currentPath,
    int currentDepth,
    const TraversalOptions& options,
    std::vector<DirectoryEntry>& results,
    std::vector<uint64_t>& visitedInodes)
{
    // Check depth limit
    if (options.maxDepth >= 0 && currentDepth > options.maxDepth) {
        return;
    }

    std::error_code ec;
    std::vector<fs::directory_entry> entries;

    // Collect entries from this directory
    try {
        for (const auto& entry : fs::directory_iterator(currentPath, ec)) {
            if (ec) {
                LOG_WARN("Error iterating: " + currentPath.string() +
                         " (" + ec.message() + ")");
                break;
            }
            entries.push_back(entry);
        }
    } catch (const fs::filesystem_error& e) {
        LOG_WARN("Cannot access directory: " + currentPath.string() +
                 " (" + e.what() + ")");
        return;  // graceful degradation: skip inaccessible dirs
    }

    // Build DirectoryEntry structs
    std::vector<DirectoryEntry> dirEntries;
    dirEntries.reserve(entries.size());

    for (const auto& fsEntry : entries) {
        auto dirEntry = buildEntry(fsEntry, currentDepth);

        // Filter hidden files
        if (!options.includeHidden && !dirEntry.name.empty() &&
            dirEntry.name[0] == '.') {
            continue;
        }

        // Apply user filter
        if (options.filter && !options.filter(dirEntry)) {
            // Still recurse into directories even if filtered out,
            // so that children can be found
            if (dirEntry.isDirectory) {
                // Check for symlink cycles
                if (dirEntry.isSymlink) {
                    if (!options.followSymlinks) {
                        continue;
                    }
                    struct stat st{};
                    if (::stat(dirEntry.path.c_str(), &st) == 0) {
                        auto ino = static_cast<uint64_t>(st.st_ino);
                        if (std::find(visitedInodes.begin(),
                                      visitedInodes.end(), ino) !=
                            visitedInodes.end()) {
                            LOG_WARN("Circular symlink detected: " +
                                     dirEntry.path);
                            continue;
                        }
                        visitedInodes.push_back(ino);
                    }
                }
                traverseRecursive(fs::path(dirEntry.path),
                                  currentDepth + 1, options,
                                  results, visitedInodes);
            }
            continue;
        }

        dirEntries.push_back(std::move(dirEntry));
    }

    // Sort if a comparator is provided
    if (options.sorter) {
        std::sort(dirEntries.begin(), dirEntries.end(), options.sorter);
    }

    // Add to results and recurse into directories
    for (auto& entry : dirEntries) {
        bool isDir = entry.isDirectory;
        bool isSym = entry.isSymlink;
        std::string entryPath = entry.path;

        results.push_back(std::move(entry));

        if (isDir) {
            // Cycle detection for symlinks
            if (isSym) {
                if (!options.followSymlinks) continue;

                struct stat st{};
                if (::stat(entryPath.c_str(), &st) == 0) {
                    auto ino = static_cast<uint64_t>(st.st_ino);
                    if (std::find(visitedInodes.begin(),
                                  visitedInodes.end(), ino) !=
                        visitedInodes.end()) {
                        LOG_WARN("Circular symlink detected: " + entryPath);
                        continue;
                    }
                    visitedInodes.push_back(ino);
                }
            }

            traverseRecursive(fs::path(entryPath),
                              currentDepth + 1, options,
                              results, visitedInodes);
        }
    }
}

// ── Build entry from filesystem ─────────────────────────────────────────────

DirectoryEntry DirectoryIterator::buildEntry(
    const fs::directory_entry& entry,
    int depth)
{
    DirectoryEntry de;
    de.path  = entry.path().string();
    de.name  = entry.path().filename().string();
    de.depth = depth;

    std::error_code ec;
    de.isSymlink     = entry.is_symlink(ec);
    de.isDirectory   = entry.is_directory(ec);
    de.isRegularFile = entry.is_regular_file(ec);

    if (de.isRegularFile) {
        de.size = static_cast<uint64_t>(entry.file_size(ec));
        if (ec) de.size = 0;
    } else {
        de.size = 0;
    }

    auto lwt = entry.last_write_time(ec);
    if (!ec) {
        // Convert file_time_type to time_t
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            lwt - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        de.modTime = std::chrono::system_clock::to_time_t(sctp);
    } else {
        de.modTime = 0;
    }

    return de;
}

} // namespace filemgr
