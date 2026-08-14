#pragma once

///////////////////////////////////////////////////////////////////////////////
// CommandDispatcher.hpp — Route commands to handler functions
///////////////////////////////////////////////////////////////////////////////

#include "CommandParser.hpp"
#include "Commands.hpp"
#include "OutputFormatter.hpp"

namespace filemgr {

class CommandDispatcher {
public:
    explicit CommandDispatcher(OutputFormatter& formatter);

    /// Dispatch a parsed command to the appropriate handler.
    /// Returns exit code (0 = success).
    int dispatch(const ParsedCommand& cmd);

private:
    Commands commands_;
    OutputFormatter& fmt_;
};

} // namespace filemgr
