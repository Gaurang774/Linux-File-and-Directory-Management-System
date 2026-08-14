#pragma once

///////////////////////////////////////////////////////////////////////////////
// CommandParser.hpp — CLI argument parser
//
// Hand-rolled parser (no external deps). Handles:
//   • Positional arguments (command, paths)
//   • Named flags (--recursive, -v, --depth N)
//   • Short flag clusters (-rv)
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <vector>

namespace filemgr {

/// Recognized commands
enum class Command {
    LS, MKDIR, RMDIR, TOUCH, RM, CAT, PWD, HELP,
    TREE, FIND, CP, MV, DU, CHMOD, STAT, WRITE,
    CD, REPL, CLEAR, EXIT,
    UNKNOWN
};

/// Parsed command line result
struct ParsedCommand {
    Command                            command = Command::UNKNOWN;
    std::vector<std::string>           positionalArgs;
    std::map<std::string, std::string> namedArgs;     // --key value
    std::map<std::string, bool>        flags;          // --flag (boolean)

    /// Check if a flag is set
    [[nodiscard]] bool hasFlag(const std::string& name) const;

    /// Get named argument value, or default
    [[nodiscard]] std::string getArg(const std::string& name,
                                     const std::string& defaultVal = "") const;

    /// Get positional argument at index, or empty
    [[nodiscard]] std::string getPositional(size_t index) const;
};

class CommandParser {
public:
    /// Parse argc/argv into a ParsedCommand
    static ParsedCommand parse(int argc, char* argv[]);

    /// Parse token vector into a ParsedCommand
    static ParsedCommand parseTokens(const std::vector<std::string>& tokens);

    /// Tokenize an input command line respecting quotes (' and ")
    static std::vector<std::string> tokenizeLine(std::string_view line);

    /// Convert command name string to enum
    static Command stringToCommand(const std::string& cmd);

    /// Get command name from enum
    static std::string commandToString(Command cmd);
};

} // namespace filemgr
