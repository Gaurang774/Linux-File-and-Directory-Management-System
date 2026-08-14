#include "CommandParser.hpp"
#include "utils/StringUtils.hpp"

#include <algorithm>

namespace filemgr {

// ── ParsedCommand helpers ───────────────────────────────────────────────────

bool ParsedCommand::hasFlag(const std::string& name) const {
    auto it = flags.find(name);
    return it != flags.end() && it->second;
}

std::string ParsedCommand::getArg(const std::string& name,
                                   const std::string& defaultVal) const {
    auto it = namedArgs.find(name);
    return (it != namedArgs.end()) ? it->second : defaultVal;
}

std::string ParsedCommand::getPositional(size_t index) const {
    return (index < positionalArgs.size()) ? positionalArgs[index] : "";
}

// ── Parser ──────────────────────────────────────────────────────────────────

ParsedCommand CommandParser::parse(int argc, char* argv[]) {
    ParsedCommand result;

    if (argc < 2) {
        result.command = Command::HELP;
        return result;
    }

    // First arg is the command
    result.command = stringToCommand(argv[1]);

    // Flags that accept a value argument
    static const std::vector<std::string> valuedFlags = {
        "--depth", "--sort", "--type", "--size", "--top",
        "--modified-before", "--modified-after", "--filter"
    };

    auto isValuedFlag = [&](const std::string& flag) {
        return std::find(valuedFlags.begin(), valuedFlags.end(), flag) !=
               valuedFlags.end();
    };

    // Parse remaining args
    for (int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
            // Long option: --flag or --key=value or --key value
            auto eqPos = arg.find('=');
            if (eqPos != std::string::npos) {
                // --key=value
                std::string key = arg.substr(0, eqPos);
                std::string val = arg.substr(eqPos + 1);
                result.namedArgs[key] = val;
            } else if (isValuedFlag(arg) && i + 1 < argc) {
                // --key value
                result.namedArgs[arg] = std::string(argv[++i]);
            } else {
                // Boolean flag
                result.flags[arg] = true;
            }
        } else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
            // Short options: -r, -v, -rv (clustered)
            for (size_t j = 1; j < arg.size(); ++j) {
                char c = arg[j];
                switch (c) {
                    case 'r': result.flags["--recursive"] = true; break;
                    case 'v': result.flags["--verbose"]   = true; break;
                    case 'q': result.flags["--quiet"]     = true; break;
                    case 'l': result.flags["--long"]      = true; break;
                    case 'a': result.flags["--all"]       = true; break;
                    case 'f': result.flags["--force"]     = true; break;
                    case 'i': result.flags["--case-insensitive"] = true; break;
                    case 'n':
                        // -n N  (short for --top N)
                        if (i + 1 < argc) {
                            result.namedArgs["--top"] = std::string(argv[++i]);
                        }
                        break;
                    default:
                        // Ignore unknown short flags
                        break;
                }
            }
        } else {
            // Positional argument
            result.positionalArgs.push_back(arg);
        }
    }

    return result;
}

ParsedCommand CommandParser::parseTokens(const std::vector<std::string>& rawTokens) {
    ParsedCommand result;
    if (rawTokens.empty()) {
        result.command = Command::UNKNOWN;
        return result;
    }

    std::vector<std::string> tokens = rawTokens;
    // Ignore leading prompt or ./file-mgr if user typed or copy-pasted it inside REPL shell
    while (!tokens.empty()) {
        std::string firstLower = StringUtils::toLower(tokens[0]);
        if (firstLower == "./file-mgr" || firstLower == "file-mgr" ||
            (firstLower.find('@') != std::string::npos && firstLower.find('$') != std::string::npos)) {
            tokens.erase(tokens.begin());
        } else {
            break;
        }
    }

    if (tokens.empty()) {
        result.command = Command::UNKNOWN;
        return result;
    }

    result.command = stringToCommand(tokens[0]);

    static const std::vector<std::string> valuedFlags = {
        "--depth", "--sort", "--type", "--size", "--top",
        "--modified-before", "--modified-after", "--filter"
    };

    auto isValuedFlag = [&](const std::string& flag) {
        return std::find(valuedFlags.begin(), valuedFlags.end(), flag) !=
               valuedFlags.end();
    };

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& arg = tokens[i];

        if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
            auto eqPos = arg.find('=');
            if (eqPos != std::string::npos) {
                std::string key = arg.substr(0, eqPos);
                std::string val = arg.substr(eqPos + 1);
                result.namedArgs[key] = val;
            } else if (isValuedFlag(arg) && i + 1 < tokens.size()) {
                result.namedArgs[arg] = tokens[++i];
            } else {
                result.flags[arg] = true;
            }
        } else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
            for (size_t j = 1; j < arg.size(); ++j) {
                char c = arg[j];
                switch (c) {
                    case 'r': result.flags["--recursive"] = true; break;
                    case 'v': result.flags["--verbose"]   = true; break;
                    case 'q': result.flags["--quiet"]     = true; break;
                    case 'l': result.flags["--long"]      = true; break;
                    case 'a': result.flags["--all"]       = true; break;
                    case 'f': result.flags["--force"]     = true; break;
                    case 'i': result.flags["--case-insensitive"] = true; break;
                    case 'n':
                        if (i + 1 < tokens.size()) {
                            result.namedArgs["--top"] = tokens[++i];
                        }
                        break;
                    default:
                        break;
                }
            }
        } else {
            result.positionalArgs.push_back(arg);
        }
    }

    return result;
}

std::vector<std::string> CommandParser::tokenizeLine(std::string_view line) {
    std::vector<std::string> tokens;
    std::string currentToken;
    bool inDoubleQuote = false;
    bool inSingleQuote = false;
    bool escaped = false;

    for (char c : line) {
        if (escaped) {
            currentToken += c;
            escaped = false;
            continue;
        }

        if (c == '\\' && !inSingleQuote) {
            escaped = true;
            continue;
        }

        if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }

        if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c)) && !inDoubleQuote && !inSingleQuote) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            continue;
        }

        currentToken += c;
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

Command CommandParser::stringToCommand(const std::string& cmd) {
    std::string lower = StringUtils::toLower(cmd);

    if (lower == "ls" || lower == "list")      return Command::LS;
    if (lower == "mkdir")                      return Command::MKDIR;
    if (lower == "rmdir")                      return Command::RMDIR;
    if (lower == "touch" || lower == "create")  return Command::TOUCH;
    if (lower == "rm" || lower == "delete")     return Command::RM;
    if (lower == "cat" || lower == "read")      return Command::CAT;
    if (lower == "pwd")                        return Command::PWD;
    if (lower == "help" || lower == "--help" ||
        lower == "-h")                         return Command::HELP;
    if (lower == "tree")                       return Command::TREE;
    if (lower == "find" || lower == "search")   return Command::FIND;
    if (lower == "cp" || lower == "copy")       return Command::CP;
    if (lower == "mv" || lower == "move")       return Command::MV;
    if (lower == "du" || lower == "usage")      return Command::DU;
    if (lower == "chmod")                      return Command::CHMOD;
    if (lower == "stat" || lower == "info")     return Command::STAT;
    if (lower == "write")                      return Command::WRITE;
    if (lower == "cd")                         return Command::CD;
    if (lower == "repl" || lower == "-i" ||
        lower == "--interactive")              return Command::REPL;
    if (lower == "clear" || lower == "cls")     return Command::CLEAR;
    if (lower == "exit" || lower == "quit" ||
        lower == "q")                          return Command::EXIT;

    return Command::UNKNOWN;
}

std::string CommandParser::commandToString(Command cmd) {
    switch (cmd) {
        case Command::LS:      return "ls";
        case Command::MKDIR:   return "mkdir";
        case Command::RMDIR:   return "rmdir";
        case Command::TOUCH:   return "touch";
        case Command::RM:      return "rm";
        case Command::CAT:     return "cat";
        case Command::PWD:     return "pwd";
        case Command::HELP:    return "help";
        case Command::TREE:    return "tree";
        case Command::FIND:    return "find";
        case Command::CP:      return "cp";
        case Command::MV:      return "mv";
        case Command::DU:      return "du";
        case Command::CHMOD:   return "chmod";
        case Command::STAT:    return "stat";
        case Command::WRITE:   return "write";
        case Command::CD:      return "cd";
        case Command::REPL:    return "repl";
        case Command::CLEAR:   return "clear";
        case Command::EXIT:    return "exit";
        case Command::UNKNOWN: return "unknown";
    }
    return "unknown";
}

} // namespace filemgr
