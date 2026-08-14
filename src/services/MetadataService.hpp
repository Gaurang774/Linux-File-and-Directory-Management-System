#pragma once

///////////////////////////////////////////////////////////////////////////////
// MetadataService.hpp — File metadata extraction
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <ctime>
#include <string>
#include <sys/types.h>

namespace filemgr {

/// File type categories
enum class FileType {
    REGULAR,
    DIRECTORY,
    SYMLINK,
    BLOCK_DEVICE,
    CHAR_DEVICE,
    FIFO,
    SOCKET,
    UNKNOWN
};

/// Complete metadata for a file or directory
struct FileMetadata {
    std::string path;
    uint64_t    size;
    time_t      modificationTime;
    time_t      accessTime;
    time_t      statusChangeTime;
    mode_t      permissions;
    uid_t       ownerUid;
    gid_t       ownerGid;
    std::string ownerName;    // resolved username
    std::string groupName;    // resolved group name
    FileType    type;
    uint64_t    hardLinks;
    uint64_t    inode;
    uint64_t    blockSize;
    uint64_t    blocks;
};

class MetadataService {
public:
    /// Get full metadata for a path.
    static FileMetadata getMetadata(const std::string& path);

    /// Get file type as enum.
    static FileType getFileType(const std::string& path);

    /// Get file type as human-readable string.
    static std::string fileTypeString(FileType type);

    /// Format metadata as multi-line stat-like output.
    static std::string formatStat(const std::string& path);
};

} // namespace filemgr
