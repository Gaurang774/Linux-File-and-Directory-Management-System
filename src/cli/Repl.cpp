#include "Repl.hpp"
#include "CommandParser.hpp"
#include "core/PathUtils.hpp"
#include "utils/Logger.hpp"
#include "utils/StringUtils.hpp"

#include <iostream>
#include <string>

namespace filemgr {

Repl::Repl(OutputFormatter& formatter)
    : fmt_(formatter), dispatcher_(formatter) {}

void Repl::run() {
    std::cout << fmt_.colorize("\n┌────────────────────────────────────────────────────────┐\n", Color::BOLD_CYAN);
    std::cout << fmt_.colorize("│    file-mgr v1.0.0 — Interactive File Management Shell   │\n", Color::BOLD_CYAN);
    std::cout << fmt_.colorize("└────────────────────────────────────────────────────────┘\n", Color::BOLD_CYAN);
    std::cout << fmt_.colorize("Type 'help' for commands, 'cd <path>' to navigate, 'exit' to quit.\n\n", Color::DIM);

    std::string line;
    while (true) {
        std::string cwd = PathUtils::currentDir();
        std::string prompt = fmt_.colorize("file-mgr ", Color::BOLD_BLUE) +
                             fmt_.colorize("[", Color::DIM) +
                             fmt_.colorize(cwd, Color::BOLD_CYAN) +
                             fmt_.colorize("]", Color::DIM) +
                             fmt_.colorize("> ", Color::BOLD);

        std::cout << prompt << std::flush;

        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            std::cout << "\n";
            break;
        }

        std::string trimmed = StringUtils::trim(line);
        if (trimmed.empty()) {
            continue;
        }

        auto tokens = CommandParser::tokenizeLine(trimmed);
        if (tokens.empty()) {
            continue;
        }

        auto cmd = CommandParser::parseTokens(tokens);

        if (cmd.command == Command::EXIT) {
            std::cout << fmt_.colorize("Goodbye!\n", Color::DIM);
            break;
        }

        dispatcher_.dispatch(cmd);
        std::cout << "\n";
    }
}

} // namespace filemgr
