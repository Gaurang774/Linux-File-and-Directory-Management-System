#include "MetadataService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"
#include "utils/TimeUtils.hpp"

#include <grp.h>
#include <pwd.h>
#include <sstream>
#include <sys/stat.h>

namespace filemgr {

FileMetadata MetadataService::getMetadata(const std::string& path) {
    PathUtils::validatePath(path);

    struct stat st{};
    if (::lstat(path.c_str(), &st) < 0) {
        int err = errno;
        if (err == ENOENT) throw FileNotFoundException(path);
        if (err == EACCES) throw PermissionDeniedException(path);
        throw IOErrorException("stat failed", err, path);
    }

    FileMetadata meta;
    meta.path               = path;
    meta.size               = static_cast<uint64_t>(st.st_size);
    meta.modificationTime   = st.st_mtime;
    meta.accessTime         = st.st_atime;
    meta.statusChangeTime   = st.st_ctime;
    meta.permissions        = st.st_mode;
    meta.ownerUid           = st.st_uid;
    meta.ownerGid           = st.st_gid;
    meta.hardLinks          = static_cast<uint64_t>(st.st_nlink);
    meta.inode              = static_cast<uint64_t>(st.st_ino);
    meta.blockSize          = static_cast<uint64_t>(st.st_blksize);
    meta.blocks             = static_cast<uint64_t>(st.st_blocks);

    // Resolve owner/group names
    struct passwd* pw = ::getpwuid(st.st_uid);
    meta.ownerName = pw ? pw->pw_name : std::to_string(st.st_uid);

    struct group* gr = ::getgrgid(st.st_gid);
    meta.groupName = gr ? gr->gr_name : std::to_string(st.st_gid);

    // Determine file type from mode
    if (S_ISREG(st.st_mode))       meta.type = FileType::REGULAR;
    else if (S_ISDIR(st.st_mode))  meta.type = FileType::DIRECTORY;
    else if (S_ISLNK(st.st_mode))  meta.type = FileType::SYMLINK;
    else if (S_ISBLK(st.st_mode))  meta.type = FileType::BLOCK_DEVICE;
    else if (S_ISCHR(st.st_mode))  meta.type = FileType::CHAR_DEVICE;
    else if (S_ISFIFO(st.st_mode)) meta.type = FileType::FIFO;
    else if (S_ISSOCK(st.st_mode)) meta.type = FileType::SOCKET;
    else                           meta.type = FileType::UNKNOWN;

    return meta;
}

FileType MetadataService::getFileType(const std::string& path) {
    return getMetadata(path).type;
}

std::string MetadataService::fileTypeString(FileType type) {
    switch (type) {
        case FileType::REGULAR:      return "regular file";
        case FileType::DIRECTORY:    return "directory";
        case FileType::SYMLINK:      return "symbolic link";
        case FileType::BLOCK_DEVICE: return "block device";
        case FileType::CHAR_DEVICE:  return "character device";
        case FileType::FIFO:         return "FIFO/pipe";
        case FileType::SOCKET:       return "socket";
        case FileType::UNKNOWN:      return "unknown";
    }
    return "unknown";
}

std::string MetadataService::formatStat(const std::string& path) {
    auto meta = getMetadata(path);

    std::ostringstream oss;
    oss << "  File: " << meta.path << "\n";
    oss << "  Size: " << meta.size
        << " (" << StringUtils::humanReadableSize(meta.size) << ")"
        << "\tBlocks: " << meta.blocks
        << "\tIO Block: " << meta.blockSize
        << "\t" << fileTypeString(meta.type) << "\n";
    oss << "Device: -"
        << "\tInode: " << meta.inode
        << "\tLinks: " << meta.hardLinks << "\n";

    // Permissions
    std::string typeChar;
    switch (meta.type) {
        case FileType::DIRECTORY:    typeChar = "d"; break;
        case FileType::SYMLINK:      typeChar = "l"; break;
        case FileType::BLOCK_DEVICE: typeChar = "b"; break;
        case FileType::CHAR_DEVICE:  typeChar = "c"; break;
        case FileType::FIFO:         typeChar = "p"; break;
        case FileType::SOCKET:       typeChar = "s"; break;
        default:                     typeChar = "-"; break;
    }

    oss << "Access: (" << StringUtils::formatOctalPermissions(meta.permissions) << "/"
        << typeChar << StringUtils::formatPermissions(meta.permissions) << ")"
        << "  Uid: (" << meta.ownerUid << "/" << meta.ownerName << ")"
        << "  Gid: (" << meta.ownerGid << "/" << meta.groupName << ")\n";

    oss << "Access: " << TimeUtils::formatTime(meta.accessTime) << "\n";
    oss << "Modify: " << TimeUtils::formatTime(meta.modificationTime) << "\n";
    oss << "Change: " << TimeUtils::formatTime(meta.statusChangeTime) << "\n";

    return oss.str();
}

} // namespace filemgr
