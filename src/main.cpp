///////////////////////////////////////////////////////////////////////////////
// main.cpp — Entry point for file-mgr
///////////////////////////////////////////////////////////////////////////////

#include "cli/CommandDispatcher.hpp"
#include "cli/CommandParser.hpp"
#include "cli/OutputFormatter.hpp"
#include "cli/Repl.hpp"
#include "utils/Logger.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    using namespace filemgr;

    // Initialize output formatter
    OutputFormatter formatter;

    // Check if interactive REPL mode requested (no args or explicitly requested)
    if (argc == 1) {
        Repl repl(formatter);
        repl.run();
        return 0;
    }

    // Parse command line
    auto cmd = CommandParser::parse(argc, argv);

    if (cmd.hasFlag("--no-color")) {
        formatter.setColorEnabled(false);
    }

    // Configure logger based on flags
    auto& logger = Logger::instance();
    if (cmd.hasFlag("--debug")) {
        logger.setLevel(LogLevel::DEBUG);
    } else if (cmd.hasFlag("--verbose")) {
        logger.setLevel(LogLevel::INFO);
    } else if (cmd.hasFlag("--quiet")) {
        logger.setLevel(LogLevel::QUIET);
    } else {
        logger.setLevel(LogLevel::ERROR);
    }

    LOG_DEBUG("file-mgr v1.0.0 started");
    LOG_DEBUG("Command: " + CommandParser::commandToString(cmd.command));

    if (cmd.command == Command::REPL) {
        Repl repl(formatter);
        repl.run();
        return 0;
    }

    // Dispatch single command
    CommandDispatcher dispatcher(formatter);
    int exitCode = dispatcher.dispatch(cmd);

    LOG_DEBUG("Exiting with code " + std::to_string(exitCode));
    return exitCode;
}
