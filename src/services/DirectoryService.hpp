#pragma once

///////////////////////////////////////////////////////////////////////////////
// DirectoryService.hpp — Directory operations
///////////////////////////////////////////////////////////////////////////////

#include "core/DirectoryIterator.hpp"

#include <string>
#include <vector>

namespace filemgr {

/// Options for listing directories
struct ListOptions {
    bool recursive   = false;
    bool longFormat   = false;   // show metadata
    bool showHidden  = false;
    int  maxDepth    = -1;       // -1 = unlimited (only for recursive)

    enum class SortBy { NAME, SIZE, DATE, NONE };
    SortBy sortBy = SortBy::NAME;

    /// Filter by extension (e.g., ".cpp")
    std::string filterExtension;

    /// Filter by name pattern (glob)
    std::string filterPattern;
};

class DirectoryService {
public:
    /// Create a directory. If recursive, creates parent directories too.
    static void createDirectory(const std::string& path, bool recursive = false);

    /// Remove an empty directory.
    static void removeDirectory(const std::string& path);

    /// Remove directory and all contents recursively.
    /// Returns count of items removed.
    /// If dryRun is true, only reports what would be removed.
    static size_t removeDirectoryRecursive(const std::string& path,
                                           bool dryRun = false);

    /// List directory contents.
    static std::vector<DirectoryEntry> listDirectory(
        const std::string& path,
        const ListOptions& options = {}
    );

    /// Copy directory recursively.
    /// Returns count of items copied.
    static size_t copyDirectory(const std::string& src,
                                const std::string& dst,
                                bool dryRun = false);

    /// Generate tree-view string for a directory.
    static std::string treeView(const std::string& path,
                                int maxDepth = -1,
                                bool showLong = false);
};

} // namespace filemgr
