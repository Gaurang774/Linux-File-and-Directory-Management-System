#include "CommandDispatcher.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <iostream>

namespace filemgr {

CommandDispatcher::CommandDispatcher(OutputFormatter& formatter)
    : commands_(formatter), fmt_(formatter) {}

int CommandDispatcher::dispatch(const ParsedCommand& cmd) {
    try {
        switch (cmd.command) {
            case Command::LS:      return commands_.cmdLs(cmd);
            case Command::MKDIR:   return commands_.cmdMkdir(cmd);
            case Command::RMDIR:   return commands_.cmdRmdir(cmd);
            case Command::TOUCH:   return commands_.cmdTouch(cmd);
            case Command::RM:      return commands_.cmdRm(cmd);
            case Command::CAT:     return commands_.cmdCat(cmd);
            case Command::WRITE:   return commands_.cmdWrite(cmd);
            case Command::PWD:     return commands_.cmdPwd(cmd);
            case Command::HELP:    return commands_.cmdHelp(cmd);
            case Command::TREE:    return commands_.cmdTree(cmd);
            case Command::FIND:    return commands_.cmdFind(cmd);
            case Command::CP:      return commands_.cmdCp(cmd);
            case Command::MV:      return commands_.cmdMv(cmd);
            case Command::DU:      return commands_.cmdDu(cmd);
            case Command::CHMOD:   return commands_.cmdChmod(cmd);
            case Command::STAT:    return commands_.cmdStat(cmd);
            case Command::CD:      return commands_.cmdCd(cmd);
            case Command::CLEAR:   return commands_.cmdClear(cmd);
            case Command::EXIT:    return 0;
            case Command::REPL:    return 0;

            case Command::UNKNOWN:
                std::cerr << fmt_.formatError(
                    "Unknown command. Run 'file-mgr help' for usage.") << "\n";
                return 1;
        }
    } catch (const FileNotFoundException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        LOG_DEBUG("FileNotFoundException: " + std::string(e.what()));
        return 1;
    } catch (const PermissionDeniedException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        LOG_DEBUG("PermissionDeniedException: " + std::string(e.what()));
        return 1;
    } catch (const DirectoryNotEmptyException& e) {
        std::cerr << fmt_.formatError(
            std::string(e.what()) +
            "\nUse 'rm -r' to remove non-empty directories.") << "\n";
        return 1;
    } catch (const PathExistsException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        return 1;
    } catch (const InvalidPathException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        return 1;
    } catch (const InvalidArgumentException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        return 1;
    } catch (const IOErrorException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        LOG_DEBUG("IOErrorException: " + std::string(e.what()));
        return 1;
    } catch (const FileMgrException& e) {
        std::cerr << fmt_.formatError(e.what()) << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << fmt_.formatError(
            std::string("Unexpected error: ") + e.what()) << "\n";
        return 2;
    }

    return 0;  // unreachable, but satisfies compiler
}

} // namespace filemgr
