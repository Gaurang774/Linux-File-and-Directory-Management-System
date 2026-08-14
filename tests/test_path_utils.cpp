///////////////////////////////////////////////////////////////////////////////
// test_path_utils.cpp — Unit tests for PathUtils
///////////////////////////////////////////////////////////////////////////////

#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"

#include <cassert>
#include <iostream>
#include <string>

using namespace filemgr;

void test_normalize() {
    std::cout << "  test_normalize... ";

    // Basic normalization (lexical)
    std::string result = PathUtils::normalize("/a/b/../c");
    assert(result == "/a/c");

    result = PathUtils::normalize("/a/./b/./c");
    assert(result == "/a/b/c");

    result = PathUtils::normalize("/a/b/../../c");
    assert(result == "/c");

    std::cout << "PASSED\n";
}

void test_join() {
    std::cout << "  test_join... ";

    assert(PathUtils::join("/a/b", "c.txt") == "/a/b/c.txt");
    assert(PathUtils::join("/a/b/", "c.txt") == "/a/b/c.txt");
    assert(PathUtils::join("/a", "b/c") == "/a/b/c");

    std::cout << "PASSED\n";
}

void test_filename() {
    std::cout << "  test_filename... ";

    assert(PathUtils::filename("/a/b/c.txt") == "c.txt");
    assert(PathUtils::filename("/a/b/c") == "c");
    assert(PathUtils::filename("c.txt") == "c.txt");
    assert(PathUtils::filename("/") == "");

    std::cout << "PASSED\n";
}

void test_extension() {
    std::cout << "  test_extension... ";

    assert(PathUtils::extension("/a/b/c.txt") == ".txt");
    assert(PathUtils::extension("/a/b/c.tar.gz") == ".gz");
    assert(PathUtils::extension("/a/b/c") == "");
    assert(PathUtils::extension(".hidden") == "");

    std::cout << "PASSED\n";
}

void test_stem() {
    std::cout << "  test_stem... ";

    assert(PathUtils::stem("/a/b/c.txt") == "c");
    assert(PathUtils::stem("/a/b/c.tar.gz") == "c.tar");
    assert(PathUtils::stem("file") == "file");

    std::cout << "PASSED\n";
}

void test_parentDir() {
    std::cout << "  test_parentDir... ";

    assert(PathUtils::parentDir("/a/b/c.txt") == "/a/b");
    assert(PathUtils::parentDir("/a/b/") == "/a/b");
    assert(PathUtils::parentDir("/a") == "/");

    std::cout << "PASSED\n";
}

void test_exists() {
    std::cout << "  test_exists... ";

    assert(PathUtils::exists("/") == true);
    assert(PathUtils::exists("/tmp") == true);
    assert(PathUtils::exists("/nonexistent_path_xyz_123") == false);

    std::cout << "PASSED\n";
}

void test_isFile_isDirectory() {
    std::cout << "  test_isFile_isDirectory... ";

    assert(PathUtils::isDirectory("/tmp") == true);
    assert(PathUtils::isFile("/tmp") == false);
    assert(PathUtils::isDirectory("/") == true);

    std::cout << "PASSED\n";
}

void test_currentDir() {
    std::cout << "  test_currentDir... ";

    std::string cwd = PathUtils::currentDir();
    assert(!cwd.empty());
    assert(cwd[0] == '/');  // absolute path

    std::cout << "PASSED\n";
}

void test_validatePath_empty() {
    std::cout << "  test_validatePath_empty... ";

    try {
        PathUtils::validatePath("");
        assert(false && "Should have thrown");
    } catch (const InvalidPathException&) {
        // expected
    }

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== PathUtils Tests ===\n";

    test_normalize();
    test_join();
    test_filename();
    test_extension();
    test_stem();
    test_parentDir();
    test_exists();
    test_isFile_isDirectory();
    test_currentDir();
    test_validatePath_empty();

    std::cout << "\nAll PathUtils tests PASSED!\n";
    return 0;
}
