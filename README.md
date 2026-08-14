# Linux File and Directory Management System (`file-mgr`)

A fast, lightweight, and extensible command-line file and directory management utility for Linux written in modern **C++17** with a **CMake** build system.

Wrapping POSIX file system operations into a clean architecture, `file-mgr` provides low-level file I/O abstractions, recursive directory traversals with circular symlink detection, find-like search, permission management, disk usage analysis, and an **Interactive REPL Shell**.

---

## 🌟 Key Features

- **File Operations**: Create (`touch`), read (`cat`), write/append (`write`), delete (`rm`), move (`mv`), copy (`cp`).
- **Directory Operations**: Create (`mkdir`), remove (`rmdir`), listing (`ls -la`), visual tree view (`tree --depth N`).
- **Interactive REPL Shell**: Run `./file-mgr` to launch an interactive session with prompt directory tracking (`cd`) and quote-aware command parsing.
- **Find & Search**: Search by glob patterns (`*.cpp`), file type (`--type f|d`), size range (`--size +10M`), date range (`--modified-before/after`), and case-sensitivity toggle.
- **Disk Usage Analysis**: `du` command with summary stats (`--summary`) and top-N largest files ranking (`--top 10`).
- **Permission & Metadata Management**: `chmod` supporting both octal (`755`) and symbolic modes (`u+x`, `go-w`), `chown`, and stat metadata display (`stat`).
- **Safety & Color**: Dry-run mode (`--dry-run`), confirmation prompts, ANSI color auto-detection (`isatty`), and verbosity levels (`-v`, `-q`, `--debug`).
- **Zero External Dependencies**: Standard C++17 library + POSIX headers.

---

## 🏗️ Architecture

```
src/
├── core/                  # POSIX wrappers (FileHandle RAII, DirectoryIterator DFS, PathUtils)
├── services/              # Stateless domain logic (File, Directory, Search, Metadata, Permission, Analytics)
├── cli/                   # User interface (CommandParser, CommandDispatcher, Commands, OutputFormatter, Repl)
├── utils/                 # Cross-cutting utilities (Exceptions hierarchy, Logger, StringUtils, TimeUtils)
└── main.cpp               # Application entry point
```

---

## 🚀 Quick Start (Build & Run in Linux / WSL)

### 1. Prerequisites
Ensure `g++` (C++17 support), `cmake`, and `make` are installed:
```bash
sudo apt-get update && sudo apt-get install -y g++ cmake make
```

### 2. Build the Project
```bash
git clone <your-repo-url>
cd "OS Mini Project"
mkdir -p build && cd build
cmake .. && make -j$(nproc)
```

### 3. Run Unit Tests
```bash
ctest --output-on-failure
```

---

## 💻 Usage Examples

### Option A: Interactive REPL Shell Mode
```bash
./file-mgr
```
```text
┌────────────────────────────────────────────────────────┐
│    file-mgr v1.0.0 — Interactive File Management Shell   │
└────────────────────────────────────────────────────────┘
file-mgr [/mnt/d/OS Mini Project/build]> touch notes.txt
file-mgr [/mnt/d/OS Mini Project/build]> write notes.txt "Hello from file-mgr!"
file-mgr [/mnt/d/OS Mini Project/build]> cat notes.txt
file-mgr [/mnt/d/OS Mini Project/build]> cd ..
file-mgr [/mnt/d/OS Mini Project]> ls -la
file-mgr [/mnt/d/OS Mini Project]> tree . --depth 2
file-mgr [/mnt/d/OS Mini Project]> exit
```

### Option B: Single CLI Commands
```bash
./file-mgr ls -la /tmp                      # Long listing including hidden files
./file-mgr tree . --depth 2                  # Visual tree representation
./file-mgr find "*.cpp" . --type f          # Search for C++ files
./file-mgr du . --top 5                     # List top 5 largest files
./file-mgr chmod 755 script.sh              # Change permissions
./file-mgr stat CMakeLists.txt              # Display detailed metadata
./file-mgr rm -r ~/temp/ --dry-run          # Safe dry-run preview
```

---

## 📄 Documentation

Check [`USAGE.txt`](USAGE.txt) for a complete list of commands, flags, and upcoming feature roadmap.

---

## 📜 License

Distributed under the MIT License. See `LICENSE` for more information.
