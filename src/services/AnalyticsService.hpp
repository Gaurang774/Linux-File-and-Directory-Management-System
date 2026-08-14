#pragma once

///////////////////////////////////////////////////////////////////////////////
// AnalyticsService.hpp — Disk usage analysis
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace filemgr {

/// Summary statistics for a directory
struct DiskUsageSummary {
    uint64_t totalSize;
    size_t   fileCount;
    size_t   directoryCount;
    size_t   symlinkCount;
};

class AnalyticsService {
public:
    /// Get size of a single file.
    static uint64_t getFileSize(const std::string& path);

    /// Get total size of a directory (recursive).
    static uint64_t getDirectorySize(const std::string& path);

    /// Get the top N largest files in a directory (recursive).
    /// Returns pairs of (path, size) sorted by size descending.
    static std::vector<std::pair<std::string, uint64_t>>
        getTopLargest(const std::string& path, size_t n);

    /// Get summary statistics for a directory.
    static DiskUsageSummary getSummary(const std::string& path);
};

} // namespace filemgr
