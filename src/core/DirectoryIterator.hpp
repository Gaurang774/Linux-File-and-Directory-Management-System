#pragma once

///////////////////////////////////////////////////////////////////////////////
// DirectoryIterator.hpp — Recursive directory traversal with filtering
//
// Features:
//   • Configurable max depth
//   • Filter predicate
//   • Sort strategy
//   • Symlink cycle detection (inode tracking)
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace filemgr {

namespace fs = std::filesystem;

/// Information about a single directory entry during traversal
struct DirectoryEntry {
    std::string  path;          // Full path
    std::string  name;          // Filename only
    bool         isDirectory;
    bool         isSymlink;
    bool         isRegularFile;
    uint64_t     size;          // 0 for directories (not recursively computed)
    time_t       modTime;       // Last modification time
    int          depth;         // Depth relative to traversal root
};

/// Options controlling directory traversal
struct TraversalOptions {
    int maxDepth = -1;  // -1 = unlimited

    /// Filter: return true to include this entry in results
    std::function<bool(const DirectoryEntry&)> filter = nullptr;

    /// Sort comparator for entries within each directory
    std::function<bool(const DirectoryEntry&, const DirectoryEntry&)> sorter = nullptr;

    bool followSymlinks = false;   // Follow symbolic links?
    bool includeHidden  = true;    // Include dotfiles?
};

/// Recursive directory traversal engine
class DirectoryIterator {
public:
    /// Traverse directory, returning all matching entries.
    /// Throws FileNotFoundException if root doesn't exist.
    /// Throws PermissionDeniedException for inaccessible directories.
    static std::vector<DirectoryEntry> traverse(
        const std::string& rootPath,
        const TraversalOptions& options = {}
    );

private:
    /// Internal recursive DFS worker
    static void traverseRecursive(
        const fs::path& currentPath,
        int currentDepth,
        const TraversalOptions& options,
        std::vector<DirectoryEntry>& results,
        std::vector<uint64_t>& visitedInodes  // cycle detection
    );

    /// Build a DirectoryEntry from a filesystem entry
    static DirectoryEntry buildEntry(
        const fs::directory_entry& entry,
        int depth
    );
};

} // namespace filemgr
