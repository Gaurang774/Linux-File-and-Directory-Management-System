#include "AnalyticsService.hpp"
#include "core/DirectoryIterator.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <algorithm>
#include <sys/stat.h>

namespace filemgr {

uint64_t AnalyticsService::getFileSize(const std::string& path) {
    PathUtils::validatePath(path);

    struct stat st{};
    if (::stat(path.c_str(), &st) < 0) {
        int err = errno;
        if (err == ENOENT) throw FileNotFoundException(path);
        if (err == EACCES) throw PermissionDeniedException(path);
        throw IOErrorException("stat failed", err, path);
    }

    return static_cast<uint64_t>(st.st_size);
}

uint64_t AnalyticsService::getDirectorySize(const std::string& path) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }
    if (!PathUtils::isDirectory(path)) {
        // Single file: return its size
        return getFileSize(path);
    }

    auto entries = DirectoryIterator::traverse(path, {});

    uint64_t totalSize = 0;
    for (const auto& entry : entries) {
        if (entry.isRegularFile) {
            totalSize += entry.size;
        }
    }

    return totalSize;
}

std::vector<std::pair<std::string, uint64_t>>
AnalyticsService::getTopLargest(const std::string& path, size_t n) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }

    TraversalOptions opts;
    opts.filter = [](const DirectoryEntry& entry) {
        return entry.isRegularFile;  // only files
    };

    auto entries = DirectoryIterator::traverse(path, opts);

    // Sort by size descending
    std::sort(entries.begin(), entries.end(),
              [](const DirectoryEntry& a, const DirectoryEntry& b) {
                  return a.size > b.size;
              });

    // Take top N
    std::vector<std::pair<std::string, uint64_t>> result;
    size_t count = std::min(n, entries.size());
    result.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        result.emplace_back(entries[i].path, entries[i].size);
    }

    return result;
}

DiskUsageSummary AnalyticsService::getSummary(const std::string& path) {
    PathUtils::validatePath(path);

    if (!PathUtils::exists(path)) {
        throw FileNotFoundException(path);
    }

    DiskUsageSummary summary{};

    if (!PathUtils::isDirectory(path)) {
        summary.totalSize = getFileSize(path);
        summary.fileCount = 1;
        return summary;
    }

    auto entries = DirectoryIterator::traverse(path, {});

    for (const auto& entry : entries) {
        if (entry.isRegularFile) {
            summary.totalSize += entry.size;
            ++summary.fileCount;
        } else if (entry.isDirectory) {
            ++summary.directoryCount;
        } else if (entry.isSymlink) {
            ++summary.symlinkCount;
        }
    }

    return summary;
}

} // namespace filemgr
