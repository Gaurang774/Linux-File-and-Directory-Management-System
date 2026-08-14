cmake_minimum_required(VERSION 3.10)
project(file-mgr
    VERSION 1.0.0
    DESCRIPTION "Linux File and Directory Management System"
    LANGUAGES CXX
)

# ── C++17 ────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Compiler warnings ───────────────────────────────────────────────────────
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wnon-virtual-dtor
    -Wold-style-cast -Wcast-align
    -Woverloaded-virtual -Wconversion
    -Wsign-conversion -Wnull-dereference
    -Wformat=2
)

# ── Source files ─────────────────────────────────────────────────────────────
set(UTIL_SOURCES
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
    src/utils/TimeUtils.cpp
)

set(CORE_SOURCES
    src/core/PathUtils.cpp
    src/core/FileHandle.cpp
    src/core/DirectoryIterator.cpp
)

set(SERVICE_SOURCES
    src/services/FileService.cpp
    src/services/DirectoryService.cpp
    src/services/MetadataService.cpp
    src/services/SearchService.cpp
    src/services/PermissionService.cpp
    src/services/AnalyticsService.cpp
)

set(CLI_SOURCES
    src/cli/OutputFormatter.cpp
    src/cli/CommandParser.cpp
    src/cli/Commands.cpp
    src/cli/CommandDispatcher.cpp
    src/cli/Repl.cpp
)

# ── Library target (for reuse in tests) ──────────────────────────────────────
add_library(filemgr_lib STATIC
    ${UTIL_SOURCES}
    ${CORE_SOURCES}
    ${SERVICE_SOURCES}
    ${CLI_SOURCES}
)

target_include_directories(filemgr_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(filemgr_lib PUBLIC
    stdc++fs   # std::filesystem on older GCC
)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(file-mgr src/main.cpp)
target_link_libraries(file-mgr PRIVATE filemgr_lib)

# ── Tests ────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
