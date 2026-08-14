///////////////////////////////////////////////////////////////////////////////
// test_search_service.cpp — Unit tests for SearchService
///////////////////////////////////////////////////////////////////////////////

#include "services/SearchService.hpp"
#include "services/FileService.hpp"
#include "services/DirectoryService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>

using namespace filemgr;

namespace fs = std::filesystem;

/// Create a test directory tree:
/// testdir/
/// ├── file1.cpp
/// ├── file2.hpp
/// ├── readme.txt
/// ├── subdir/
/// │   ├── deep.cpp
/// │   └── data.bin
/// └── empty/
std::string createSearchTestDir() {
    std::string dir = "/tmp/filemgr_search_test_" + std::to_string(getpid());
    fs::create_directories(dir + "/subdir");
    fs::create_directories(dir + "/empty");

    FileService::writeFile(dir + "/file1.cpp", "// C++ source\n");
    FileService::writeFile(dir + "/file2.hpp", "// C++ header\n");
    FileService::writeFile(dir + "/readme.txt", "Read me\n");
    FileService::writeFile(dir + "/subdir/deep.cpp", "// Deep file\n");
    // Create a larger file for size filtering
    FileService::writeFile(dir + "/subdir/data.bin",
                           std::string(10000, 'X'));

    return dir;
}

void cleanupSearchTestDir(const std::string& dir) {
    std::error_code ec;
    fs::remove_all(dir, ec);
}

void test_searchByName() {
    std::cout << "  test_searchByName... ";
    auto dir = createSearchTestDir();

    SearchCriteria criteria;
    criteria.namePattern = "*.cpp";

    auto results = SearchService::search(dir, criteria);
    assert(results.size() == 2);  // file1.cpp, deep.cpp

    // Check that all results are .cpp files
    for (const auto& r : results) {
        assert(r.name.size() >= 4);
        assert(r.name.substr(r.name.size() - 4) == ".cpp");
    }

    cleanupSearchTestDir(dir);
    std::cout << "PASSED\n";
}

void test_searchByType() {
    std::cout << "  test_searchByType... ";
    auto dir = createSearchTestDir();

    // Search for directories only
    SearchCriteria criteria;
    criteria.directoriesOnly = true;

    auto results = SearchService::search(dir, criteria);
    assert(results.size() == 2);  // subdir, empty

    for (const auto& r : results) {
        assert(r.isDirectory);
    }

    // Search for files only
    criteria.directoriesOnly = false;
    results = SearchService::search(dir, criteria);
    for (const auto& r : results) {
        assert(!r.isDirectory);
    }

    cleanupSearchTestDir(dir);
    std::cout << "PASSED\n";
}

void test_searchBySize() {
    std::cout << "  test_searchBySize... ";
    auto dir = createSearchTestDir();

    // Files larger than 5000 bytes
    SearchCriteria criteria;
    criteria.minSize = 5000;

    auto results = SearchService::search(dir, criteria);
    assert(results.size() == 1);  // data.bin (10000 bytes)
    assert(results[0].name == "data.bin");

    // Files smaller than 100 bytes
    criteria.minSize = std::nullopt;
    criteria.maxSize = 100;

    results = SearchService::search(dir, criteria);
    assert(results.size() >= 2);  // small text files

    cleanupSearchTestDir(dir);
    std::cout << "PASSED\n";
}

void test_searchCombined() {
    std::cout << "  test_searchCombined... ";
    auto dir = createSearchTestDir();

    // Search for .cpp files that are small
    SearchCriteria criteria;
    criteria.namePattern = "*.cpp";
    criteria.maxSize = 100;

    auto results = SearchService::search(dir, criteria);
    assert(results.size() == 2);  // file1.cpp and deep.cpp (both small)

    cleanupSearchTestDir(dir);
    std::cout << "PASSED\n";
}

void test_searchNotFound() {
    std::cout << "  test_searchNotFound... ";
    auto dir = createSearchTestDir();

    SearchCriteria criteria;
    criteria.namePattern = "*.xyz";

    auto results = SearchService::search(dir, criteria);
    assert(results.empty());

    cleanupSearchTestDir(dir);
    std::cout << "PASSED\n";
}

void test_searchInvalidPath() {
    std::cout << "  test_searchInvalidPath... ";

    SearchCriteria criteria;
    criteria.namePattern = "*";

    try {
        SearchService::search("/nonexistent_filemgr_xyz", criteria);
        assert(false && "Should have thrown");
    } catch (const FileNotFoundException&) {
        // expected
    }

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== SearchService Tests ===\n";

    test_searchByName();
    test_searchByType();
    test_searchBySize();
    test_searchCombined();
    test_searchNotFound();
    test_searchInvalidPath();

    std::cout << "\nAll SearchService tests PASSED!\n";
    return 0;
}
