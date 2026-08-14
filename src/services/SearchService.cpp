#include "SearchService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"

#include <regex>

namespace filemgr {

std::vector<SearchResult> SearchService::search(
    const std::string& rootPath,
    const SearchCriteria& criteria)
{
    PathUtils::validatePath(rootPath);

    if (!PathUtils::exists(rootPath)) {
        throw FileNotFoundException(rootPath);
    }
    if (!PathUtils::isDirectory(rootPath)) {
        throw InvalidPathException(rootPath, "Search root must be a directory");
    }

    // Build regex from glob pattern if provided
    std::regex nameRegex;
    bool hasNameFilter = false;
    if (criteria.namePattern.has_value()) {
        std::string regexStr = StringUtils::globToRegex(criteria.namePattern.value());
        auto flags = std::regex::ECMAScript;
        if (!criteria.caseSensitive) {
            flags |= std::regex::icase;
        }
        nameRegex = std::regex(regexStr, flags);
        hasNameFilter = true;
    }

    // Set up traversal with a combined filter
    TraversalOptions opts;
    opts.maxDepth = criteria.maxDepth;

    opts.filter = [&](const DirectoryEntry& entry) -> bool {
        // Type filter
        if (criteria.directoriesOnly.has_value()) {
            if (criteria.directoriesOnly.value() && !entry.isDirectory) return false;
            if (!criteria.directoriesOnly.value() && entry.isDirectory) return false;
        }

        // Name pattern filter
        if (hasNameFilter) {
            std::string nameToMatch = criteria.caseSensitive
                ? entry.name
                : StringUtils::toLower(entry.name);
            if (!std::regex_match(entry.name, nameRegex)) return false;
        }

        // Size filters (only for regular files)
        if (entry.isRegularFile) {
            if (criteria.minSize.has_value() && entry.size < criteria.minSize.value()) {
                return false;
            }
            if (criteria.maxSize.has_value() && entry.size > criteria.maxSize.value()) {
                return false;
            }
        } else if (criteria.minSize.has_value() || criteria.maxSize.has_value()) {
            // If size filter is set and entry is not a file, skip
            // unless directoriesOnly is explicitly true
            if (!criteria.directoriesOnly.has_value() ||
                !criteria.directoriesOnly.value()) {
                return false;
            }
        }

        // Date filters
        if (criteria.modifiedAfter.has_value() &&
            entry.modTime < criteria.modifiedAfter.value()) {
            return false;
        }
        if (criteria.modifiedBefore.has_value() &&
            entry.modTime > criteria.modifiedBefore.value()) {
            return false;
        }

        return true;
    };

    // Traverse and convert
    auto entries = DirectoryIterator::traverse(rootPath, opts);

    std::vector<SearchResult> results;
    results.reserve(entries.size());

    for (const auto& entry : entries) {
        SearchResult result;
        result.path        = entry.path;
        result.name        = entry.name;
        result.isDirectory = entry.isDirectory;
        result.size        = entry.size;
        result.modTime     = entry.modTime;
        results.push_back(std::move(result));
    }

    LOG_INFO("Search found " + std::to_string(results.size()) +
             " results in: " + rootPath);
    return results;
}

} // namespace filemgr
