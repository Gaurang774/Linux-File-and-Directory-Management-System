#pragma once

///////////////////////////////////////////////////////////////////////////////
// Commands.hpp — Individual command handler implementations
///////////////////////////////////////////////////////////////////////////////

#include "CommandParser.hpp"
#include "OutputFormatter.hpp"

namespace filemgr {

class Commands {
public:
    explicit Commands(OutputFormatter& formatter) : fmt_(formatter) {}

    int cmdLs(const ParsedCommand& cmd);
    int cmdMkdir(const ParsedCommand& cmd);
    int cmdRmdir(const ParsedCommand& cmd);
    int cmdTouch(const ParsedCommand& cmd);
    int cmdRm(const ParsedCommand& cmd);
    int cmdCat(const ParsedCommand& cmd);
    int cmdWrite(const ParsedCommand& cmd);
    int cmdPwd(const ParsedCommand& cmd);
    int cmdHelp(const ParsedCommand& cmd);
    int cmdTree(const ParsedCommand& cmd);
    int cmdFind(const ParsedCommand& cmd);
    int cmdCp(const ParsedCommand& cmd);
    int cmdMv(const ParsedCommand& cmd);
    int cmdDu(const ParsedCommand& cmd);
    int cmdChmod(const ParsedCommand& cmd);
    int cmdStat(const ParsedCommand& cmd);
    int cmdCd(const ParsedCommand& cmd);
    int cmdClear(const ParsedCommand& cmd);

private:
    OutputFormatter& fmt_;

    /// Parse size string like "+100M", "-5K", "1G" into bytes.
    /// Returns 0 on parse failure.
    static uint64_t parseSizeArg(const std::string& sizeStr);
};

} // namespace filemgr
