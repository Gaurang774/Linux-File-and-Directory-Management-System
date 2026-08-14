#pragma once

///////////////////////////////////////////////////////////////////////////////
// SearchService.hpp — File search with filtering
///////////////////////////////////////////////////////////////////////////////

#include "core/DirectoryIterator.hpp"

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

namespace filemgr {

/// Search filter criteria
struct SearchCriteria {
    std::optional<std::string> namePattern;     // glob pattern
    std::optional<bool>        directoriesOnly; // true=dirs, false=files
    std::optional<uint64_t>    minSize;         // bytes
    std::optional<uint64_t>    maxSize;         // bytes
    std::optional<time_t>      modifiedAfter;
    std::optional<time_t>      modifiedBefore;
    bool                       caseSensitive = true;
    int                        maxDepth      = -1;  // -1 = unlimited
};

/// A search result entry
struct SearchResult {
    std::string path;
    std::string name;
    bool        isDirectory;
    uint64_t    size;
    time_t      modTime;
};

class SearchService {
public:
    /// Search for files/directories matching criteria.
    static std::vector<SearchResult> search(
        const std::string& rootPath,
        const SearchCriteria& criteria
    );
};

} // namespace filemgr
