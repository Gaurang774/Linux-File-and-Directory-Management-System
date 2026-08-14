#include "Commands.hpp"

#include "core/PathUtils.hpp"
#include "services/AnalyticsService.hpp"
#include "services/DirectoryService.hpp"
#include "services/FileService.hpp"
#include "services/MetadataService.hpp"
#include "services/PermissionService.hpp"
#include "services/SearchService.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"
#include "utils/TimeUtils.hpp"

#include <iostream>

namespace filemgr {

// ── ls ──────────────────────────────────────────────────────────────────────

int Commands::cmdLs(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    if (path.empty()) path = ".";

    ListOptions opts;
    opts.recursive    = cmd.hasFlag("--recursive");
    opts.longFormat   = cmd.hasFlag("--long");
    opts.showHidden   = cmd.hasFlag("--all");
    opts.maxDepth     = -1;

    // Sort option
    std::string sortStr = cmd.getArg("--sort", "name");
    if (sortStr == "size")      opts.sortBy = ListOptions::SortBy::SIZE;
    else if (sortStr == "date") opts.sortBy = ListOptions::SortBy::DATE;
    else if (sortStr == "none") opts.sortBy = ListOptions::SortBy::NONE;
    else                        opts.sortBy = ListOptions::SortBy::NAME;

    // Filter option
    opts.filterPattern = cmd.getArg("--filter");

    auto entries = DirectoryService::listDirectory(path, opts);

    if (opts.longFormat) {
        std::cout << fmt_.formatLongListing(entries);
    } else {
        std::cout << fmt_.formatSimpleListing(entries);
    }

    return 0;
}

// ── mkdir ───────────────────────────────────────────────────────────────────

int Commands::cmdMkdir(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    if (path.empty()) {
        std::cerr << fmt_.formatError("mkdir requires a path argument") << "\n";
        return 1;
    }

    bool recursive = cmd.hasFlag("--recursive");
    DirectoryService::createDirectory(path, recursive);

    if (!cmd.hasFlag("--quiet")) {
        std::cout << fmt_.formatSuccess("Created directory: " + path) << "\n";
    }
    return 0;
}

// ── rmdir ───────────────────────────────────────────────────────────────────

int Commands::cmdRmdir(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    if (path.empty()) {
        std::cerr << fmt_.formatError("rmdir requires a path argument") << "\n";
        return 1;
    }

    DirectoryService::removeDirectory(path);

    if (!cmd.hasFlag("--quiet")) {
        std::cout << fmt_.formatSuccess("Removed directory: " + path) << "\n";
    }
    return 0;
}

// ── touch ───────────────────────────────────────────────────────────────────

int Commands::cmdTouch(const ParsedCommand& cmd) {
    if (cmd.positionalArgs.empty()) {
        std::cerr << fmt_.formatError("touch requires at least one file argument") << "\n";
        return 1;
    }

    for (const auto& path : cmd.positionalArgs) {
        FileService::createFile(path);
        if (!cmd.hasFlag("--quiet")) {
            std::cout << fmt_.formatSuccess("Touched: " + path) << "\n";
        }
    }
    return 0;
}

// ── rm ──────────────────────────────────────────────────────────────────────

int Commands::cmdRm(const ParsedCommand& cmd) {
    if (cmd.positionalArgs.empty()) {
        std::cerr << fmt_.formatError("rm requires at least one file argument") << "\n";
        return 1;
    }

    bool recursive = cmd.hasFlag("--recursive");
    bool dryRun    = cmd.hasFlag("--dry-run");
    bool force     = cmd.hasFlag("--force");

    for (const auto& path : cmd.positionalArgs) {
        if (PathUtils::isDirectory(path)) {
            if (!recursive) {
                std::cerr << fmt_.formatError(
                    path + " is a directory. Use -r to remove recursively.") << "\n";
                if (!force) return 1;
                continue;
            }

            if (dryRun) {
                auto count = DirectoryService::removeDirectoryRecursive(path, true);
                std::cout << fmt_.formatWarning(
                    "[DRY RUN] Would remove " + std::to_string(count) +
                    " items from: " + path) << "\n";
            } else {
                // Confirmation prompt unless --force
                if (!force) {
                    std::cout << "Remove " << path << " and all contents? [y/N] ";
                    std::string answer;
                    std::getline(std::cin, answer);
                    if (answer != "y" && answer != "Y") {
                        std::cout << "Skipped: " << path << "\n";
                        continue;
                    }
                }
                auto count = DirectoryService::removeDirectoryRecursive(path, false);
                if (!cmd.hasFlag("--quiet")) {
                    std::cout << fmt_.formatSuccess(
                        "Removed " + std::to_string(count) + " items: " + path) << "\n";
                }
            }
        } else {
            if (dryRun) {
                std::cout << fmt_.formatWarning(
                    "[DRY RUN] Would remove: " + path) << "\n";
            } else {
                FileService::deleteFile(path);
                if (!cmd.hasFlag("--quiet")) {
                    std::cout << fmt_.formatSuccess("Removed: " + path) << "\n";
                }
            }
        }
    }
    return 0;
}

// ── cat ─────────────────────────────────────────────────────────────────────

int Commands::cmdCat(const ParsedCommand& cmd) {
    if (cmd.positionalArgs.empty()) {
        std::cerr << fmt_.formatError("cat requires a file argument") << "\n";
        return 1;
    }

    for (const auto& path : cmd.positionalArgs) {
        std::string content = FileService::readFile(path);
        std::cout << content;
    }
    return 0;
}

// ── write ───────────────────────────────────────────────────────────────────

int Commands::cmdWrite(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    std::string content = cmd.getPositional(1);

    if (path.empty()) {
        std::cerr << fmt_.formatError("write requires: <file> <content>") << "\n";
        return 1;
    }

    if (content.empty()) {
        // Read from stdin
        std::string line;
        while (std::getline(std::cin, line)) {
            content += line + "\n";
        }
    }

    bool append = cmd.hasFlag("--append");
    if (append) {
        FileService::appendFile(path, content);
    } else {
        FileService::writeFile(path, content);
    }

    if (!cmd.hasFlag("--quiet")) {
        std::cout << fmt_.formatSuccess(
            (append ? "Appended to: " : "Wrote to: ") + path) << "\n";
    }
    return 0;
}

// ── pwd ─────────────────────────────────────────────────────────────────────

int Commands::cmdPwd(const ParsedCommand& /*cmd*/) {
    std::cout << PathUtils::currentDir() << "\n";
    return 0;
}

// ── help ────────────────────────────────────────────────────────────────────

int Commands::cmdHelp(const ParsedCommand& /*cmd*/) {
    std::cout << R"(
file-mgr — Linux File and Directory Management System

USAGE:
    file-mgr <command> [arguments] [options]

COMMANDS:
    ls [path]                        List directory contents
    mkdir <path>                     Create directory
    rmdir <path>                     Remove empty directory
    touch <file> [file2...]          Create/touch files
    rm <file> [file2...]             Remove files
    cat <file> [file2...]            Display file contents
    write <file> [content]           Write to file (reads stdin if no content)
    pwd                              Print working directory
    tree [path]                      Tree view of directory
    find <pattern> [path]            Search for files
    cp <source> <dest>               Copy file or directory
    mv <source> <dest>               Move/rename file or directory
    du [path]                        Disk usage analysis
    chmod <mode> <path>              Change file permissions
    stat <path>                      Display file metadata
    help                             Show this help message

OPTIONS:
    -r, --recursive                  Operate recursively
    -l, --long                       Long listing format
    -a, --all                        Show hidden files
    -v, --verbose                    Verbose output
    -q, --quiet                      Suppress non-error output
    -f, --force                      Force operation (no prompts)
    -i, --case-insensitive           Case-insensitive search
    --no-color                       Disable colored output
    --debug                          Debug logging
    --dry-run                        Show what would be done
    --depth <N>                      Limit recursion depth
    --sort <name|size|date>          Sort listing
    --type <f|d>                     Filter by type (file/directory)
    --size <[+|-]N[B|K|M|G]>        Filter by size
    --top <N>                        Show top N results
    --modified-before <YYYY-MM-DD>   Filter by date
    --modified-after <YYYY-MM-DD>    Filter by date
    --filter <pattern>               Filter by glob pattern
    --append                         Append to file (with write)

EXAMPLES:
    file-mgr ls -la ~/Documents
    file-mgr tree ~/project --depth 2
    file-mgr find "*.cpp" ~/src --type f
    file-mgr cp -r ~/src ~/backup/
    file-mgr du ~/Downloads --top 10
    file-mgr chmod 755 script.sh
    file-mgr stat important.txt
    file-mgr rm -r --dry-run ~/temp/

)" << std::flush;
    return 0;
}

// ── tree ────────────────────────────────────────────────────────────────────

int Commands::cmdTree(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    if (path.empty()) path = ".";

    int depth = -1;
    std::string depthStr = cmd.getArg("--depth");
    if (!depthStr.empty()) {
        try { depth = std::stoi(depthStr); }
        catch (...) {
            std::cerr << fmt_.formatError("Invalid depth: " + depthStr) << "\n";
            return 1;
        }
    }

    bool showLong = cmd.hasFlag("--long");
    std::string tree = DirectoryService::treeView(path, depth, showLong);
    std::cout << tree;
    return 0;
}

// ── find ────────────────────────────────────────────────────────────────────

int Commands::cmdFind(const ParsedCommand& cmd) {
    std::string pattern = cmd.getPositional(0);
    std::string searchPath = cmd.getPositional(1);
    if (searchPath.empty()) searchPath = ".";

    SearchCriteria criteria;
    if (!pattern.empty()) {
        criteria.namePattern = pattern;
    }

    // Type filter
    std::string typeStr = cmd.getArg("--type");
    if (typeStr == "f" || typeStr == "file") {
        criteria.directoriesOnly = false;
    } else if (typeStr == "d" || typeStr == "dir" || typeStr == "directory") {
        criteria.directoriesOnly = true;
    }

    // Size filter
    std::string sizeStr = cmd.getArg("--size");
    if (!sizeStr.empty()) {
        bool isMin = false;
        if (sizeStr[0] == '+') {
            isMin = true;
            sizeStr = sizeStr.substr(1);
        } else if (sizeStr[0] == '-') {
            sizeStr = sizeStr.substr(1);
        }

        uint64_t sizeBytes = parseSizeArg(sizeStr);
        if (sizeBytes > 0) {
            if (isMin) criteria.minSize = sizeBytes;
            else       criteria.maxSize = sizeBytes;
        }
    }

    // Date filters
    std::string modBefore = cmd.getArg("--modified-before");
    if (!modBefore.empty()) {
        time_t t = TimeUtils::parseDate(modBefore);
        if (t != static_cast<time_t>(-1)) criteria.modifiedBefore = t;
    }

    std::string modAfter = cmd.getArg("--modified-after");
    if (!modAfter.empty()) {
        time_t t = TimeUtils::parseDate(modAfter);
        if (t != static_cast<time_t>(-1)) criteria.modifiedAfter = t;
    }

    // Case sensitivity
    criteria.caseSensitive = !cmd.hasFlag("--case-insensitive");

    // Depth
    std::string depthStr = cmd.getArg("--depth");
    if (!depthStr.empty()) {
        try { criteria.maxDepth = std::stoi(depthStr); }
        catch (...) { /* ignore bad depth */ }
    }

    auto results = SearchService::search(searchPath, criteria);

    for (const auto& result : results) {
        if (cmd.hasFlag("--long")) {
            std::cout << fmt_.formatLongEntry(result.path) << "\n";
        } else {
            std::cout << result.path << "\n";
        }
    }

    if (!cmd.hasFlag("--quiet")) {
        std::cerr << "\n" << results.size() << " result(s) found.\n";
    }
    return 0;
}

// ── cp ──────────────────────────────────────────────────────────────────────

int Commands::cmdCp(const ParsedCommand& cmd) {
    std::string src = cmd.getPositional(0);
    std::string dst = cmd.getPositional(1);

    if (src.empty() || dst.empty()) {
        std::cerr << fmt_.formatError("cp requires: <source> <destination>") << "\n";
        return 1;
    }

    bool recursive = cmd.hasFlag("--recursive");
    bool dryRun    = cmd.hasFlag("--dry-run");

    if (PathUtils::isDirectory(src)) {
        if (!recursive) {
            std::cerr << fmt_.formatError(
                src + " is a directory. Use -r to copy recursively.") << "\n";
            return 1;
        }
        auto count = DirectoryService::copyDirectory(src, dst, dryRun);
        if (!cmd.hasFlag("--quiet")) {
            if (dryRun) {
                std::cout << fmt_.formatWarning(
                    "[DRY RUN] Would copy " + std::to_string(count) +
                    " items") << "\n";
            } else {
                std::cout << fmt_.formatSuccess(
                    "Copied " + std::to_string(count) + " items: " +
                    src + " → " + dst) << "\n";
            }
        }
    } else {
        if (dryRun) {
            std::cout << fmt_.formatWarning(
                "[DRY RUN] Would copy: " + src + " → " + dst) << "\n";
        } else {
            FileService::copyFile(src, dst, cmd.hasFlag("--force"));
            if (!cmd.hasFlag("--quiet")) {
                std::cout << fmt_.formatSuccess(
                    "Copied: " + src + " → " + dst) << "\n";
            }
        }
    }
    return 0;
}

// ── mv ──────────────────────────────────────────────────────────────────────

int Commands::cmdMv(const ParsedCommand& cmd) {
    std::string src = cmd.getPositional(0);
    std::string dst = cmd.getPositional(1);

    if (src.empty() || dst.empty()) {
        std::cerr << fmt_.formatError("mv requires: <source> <destination>") << "\n";
        return 1;
    }

    bool dryRun = cmd.hasFlag("--dry-run");

    if (dryRun) {
        std::cout << fmt_.formatWarning(
            "[DRY RUN] Would move: " + src + " → " + dst) << "\n";
    } else {
        FileService::moveFile(src, dst);
        if (!cmd.hasFlag("--quiet")) {
            std::cout << fmt_.formatSuccess(
                "Moved: " + src + " → " + dst) << "\n";
        }
    }
    return 0;
}

// ── du ──────────────────────────────────────────────────────────────────────

int Commands::cmdDu(const ParsedCommand& cmd) {
    std::string path = cmd.getPositional(0);
    if (path.empty()) path = ".";

    bool summary = cmd.hasFlag("--summary");
    std::string topStr = cmd.getArg("--top");

    if (!topStr.empty()) {
        // Show top N largest files
        size_t n = 10;
        try { n = static_cast<size_t>(std::stoi(topStr)); }
        catch (...) { /* default 10 */ }

        auto top = AnalyticsService::getTopLargest(path, n);

        std::cout << fmt_.colorize("Top " + std::to_string(top.size()) +
                                   " largest files in: " + path + "\n",
                                   Color::BOLD) << "\n";

        for (size_t i = 0; i < top.size(); ++i) {
            std::cout << StringUtils::padLeft(std::to_string(i + 1), 4) << ". "
                      << StringUtils::padLeft(
                             StringUtils::humanReadableSize(top[i].second), 10)
                      << "  " << top[i].first << "\n";
        }
        return 0;
    }

    if (summary || !PathUtils::isDirectory(path)) {
        auto summ = AnalyticsService::getSummary(path);
        std::cout << fmt_.colorize("Disk Usage Summary: " + path + "\n",
                                   Color::BOLD);
        std::cout << "  Total size:   "
                  << StringUtils::humanReadableSize(summ.totalSize) << "\n";
        std::cout << "  Files:        " << summ.fileCount << "\n";
        std::cout << "  Directories:  " << summ.directoryCount << "\n";
        std::cout << "  Symlinks:     " << summ.symlinkCount << "\n";
    } else {
        // Show directory size
        uint64_t size = AnalyticsService::getDirectorySize(path);
        std::cout << StringUtils::humanReadableSize(size) << "\t" << path << "\n";
    }
    return 0;
}

// ── chmod ───────────────────────────────────────────────────────────────────

int Commands::cmdChmod(const ParsedCommand& cmd) {
    std::string modeStr = cmd.getPositional(0);
    std::string path    = cmd.getPositional(1);

    if (modeStr.empty() || path.empty()) {
        std::cerr << fmt_.formatError("chmod requires: <mode> <path>") << "\n";
        return 1;
    }

    mode_t mode;
    // Check if it's a symbolic mode (contains +, -, =)
    if (modeStr.find_first_of("+-=") != std::string::npos) {
        auto currentPerms = PermissionService::getPermissions(path);
        mode = PermissionService::parseSymbolicMode(modeStr, currentPerms.mode);
    } else {
        mode = PermissionService::parseMode(modeStr);
    }

    PermissionService::setPermissions(path, mode);

    if (!cmd.hasFlag("--quiet")) {
        auto newPerms = PermissionService::getPermissions(path);
        std::cout << fmt_.formatSuccess(
            "Changed permissions: " + path + " → " +
            newPerms.typeChar + newPerms.rwxString +
            " (" + newPerms.octalString + ")") << "\n";
    }
    return 0;
}

// ── stat ────────────────────────────────────────────────────────────────────

int Commands::cmdStat(const ParsedCommand& cmd) {
    if (cmd.positionalArgs.empty()) {
        std::cerr << fmt_.formatError("stat requires a path argument") << "\n";
        return 1;
    }

    for (const auto& path : cmd.positionalArgs) {
        std::cout << MetadataService::formatStat(path) << "\n";
    }
    return 0;
}

// ── cd ──────────────────────────────────────────────────────────────────────

int Commands::cmdCd(const ParsedCommand& cmd) {
    std::string target = cmd.getPositional(0);
    if (target.empty()) {
        const char* home = std::getenv("HOME");
        if (home) target = home;
        else target = "/";
    }

    std::error_code ec;
    std::filesystem::current_path(target, ec);
    if (ec) {
        std::cerr << fmt_.formatError("cd: " + target + ": No such directory or permission denied") << "\n";
        return 1;
    }
    return 0;
}

// ── clear ───────────────────────────────────────────────────────────────────

int Commands::cmdClear(const ParsedCommand& /*cmd*/) {
    // ANSI clear screen code
    std::cout << "\033[2J\033[1;1H" << std::flush;
    return 0;
}

// ── Size parser ─────────────────────────────────────────────────────────────

uint64_t Commands::parseSizeArg(const std::string& sizeStr) {
    if (sizeStr.empty()) return 0;

    std::string numPart;
    uint64_t multiplier = 1;

    // Find the unit suffix
    size_t i = 0;
    while (i < sizeStr.size() && (std::isdigit(sizeStr[i]) || sizeStr[i] == '.')) {
        numPart += sizeStr[i];
        ++i;
    }

    if (i < sizeStr.size()) {
        char unit = static_cast<char>(std::toupper(sizeStr[i]));
        switch (unit) {
            case 'B': multiplier = 1; break;
            case 'K': multiplier = 1024ULL; break;
            case 'M': multiplier = 1024ULL * 1024; break;
            case 'G': multiplier = 1024ULL * 1024 * 1024; break;
            case 'T': multiplier = 1024ULL * 1024 * 1024 * 1024; break;
            default: break;
        }
    }

    try {
        double val = std::stod(numPart);
        return static_cast<uint64_t>(val * static_cast<double>(multiplier));
    } catch (...) {
        return 0;
    }
}

} // namespace filemgr
