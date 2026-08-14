#pragma once

///////////////////////////////////////////////////////////////////////////////
// Repl.hpp — Interactive REPL mode for file-mgr
///////////////////////////////////////////////////////////////////////////////

#include "CommandDispatcher.hpp"
#include "OutputFormatter.hpp"

namespace filemgr {

class Repl {
public:
    explicit Repl(OutputFormatter& formatter);

    /// Run the interactive shell loop until exit/quit or EOF.
    void run();

private:
    OutputFormatter& fmt_;
    CommandDispatcher dispatcher_;
};

} // namespace filemgr
