///////////////////////////////////////////////////////////////////////////////
// test_file_service.cpp — Unit tests for FileService
///////////////////////////////////////////////////////////////////////////////

#include "services/FileService.hpp"
#include "services/DirectoryService.hpp"
#include "core/PathUtils.hpp"
#include "utils/Exceptions.hpp"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace filemgr;

namespace fs = std::filesystem;

/// Create a temporary test directory
std::string createTestDir() {
    std::string dir = "/tmp/filemgr_test_" + std::to_string(getpid());
    fs::create_directories(dir);
    return dir;
}

/// Cleanup test directory
void cleanupTestDir(const std::string& dir) {
    std::error_code ec;
    fs::remove_all(dir, ec);
}

void test_createFile() {
    std::cout << "  test_createFile... ";
    auto testDir = createTestDir();

    std::string path = testDir + "/newfile.txt";
    FileService::createFile(path);
    assert(PathUtils::exists(path));
    assert(PathUtils::isFile(path));

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_readWriteFile() {
    std::cout << "  test_readWriteFile... ";
    auto testDir = createTestDir();

    std::string path = testDir + "/rw_test.txt";
    std::string content = "Hello, file-mgr!\nLine 2.\n";

    FileService::writeFile(path, content);
    std::string readBack = FileService::readFile(path);
    assert(readBack == content);

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_appendFile() {
    std::cout << "  test_appendFile... ";
    auto testDir = createTestDir();

    std::string path = testDir + "/append_test.txt";
    FileService::writeFile(path, "Line 1\n");
    FileService::appendFile(path, "Line 2\n");

    std::string content = FileService::readFile(path);
    assert(content == "Line 1\nLine 2\n");

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_deleteFile() {
    std::cout << "  test_deleteFile... ";
    auto testDir = createTestDir();

    std::string path = testDir + "/todelete.txt";
    FileService::createFile(path);
    assert(PathUtils::exists(path));

    FileService::deleteFile(path);
    assert(!PathUtils::exists(path));

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_deleteFile_notFound() {
    std::cout << "  test_deleteFile_notFound... ";

    try {
        FileService::deleteFile("/tmp/nonexistent_filemgr_test_xyz");
        assert(false && "Should have thrown");
    } catch (const FileNotFoundException&) {
        // expected
    }

    std::cout << "PASSED\n";
}

void test_copyFile() {
    std::cout << "  test_copyFile... ";
    auto testDir = createTestDir();

    std::string src = testDir + "/source.txt";
    std::string dst = testDir + "/copy.txt";

    FileService::writeFile(src, "Copy me!");
    FileService::copyFile(src, dst);

    assert(PathUtils::exists(dst));
    assert(FileService::readFile(dst) == "Copy me!");
    assert(PathUtils::exists(src)); // original still exists

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_copyFile_exists() {
    std::cout << "  test_copyFile_exists... ";
    auto testDir = createTestDir();

    std::string src = testDir + "/src.txt";
    std::string dst = testDir + "/dst.txt";

    FileService::writeFile(src, "Source");
    FileService::writeFile(dst, "Existing");

    try {
        FileService::copyFile(src, dst, false);  // no overwrite
        assert(false && "Should have thrown");
    } catch (const PathExistsException&) {
        // expected
    }

    // Overwrite should work
    FileService::copyFile(src, dst, true);
    assert(FileService::readFile(dst) == "Source");

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_moveFile() {
    std::cout << "  test_moveFile... ";
    auto testDir = createTestDir();

    std::string src = testDir + "/moveme.txt";
    std::string dst = testDir + "/moved.txt";

    FileService::writeFile(src, "Move me!");
    FileService::moveFile(src, dst);

    assert(!PathUtils::exists(src));
    assert(PathUtils::exists(dst));
    assert(FileService::readFile(dst) == "Move me!");

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_emptyFile() {
    std::cout << "  test_emptyFile... ";
    auto testDir = createTestDir();

    std::string path = testDir + "/empty.txt";
    FileService::createFile(path);

    std::string content = FileService::readFile(path);
    assert(content.empty());

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_mkdir_and_rmdir() {
    std::cout << "  test_mkdir_and_rmdir... ";
    auto testDir = createTestDir();

    std::string dir = testDir + "/subdir";
    DirectoryService::createDirectory(dir);
    assert(PathUtils::isDirectory(dir));

    DirectoryService::removeDirectory(dir);
    assert(!PathUtils::exists(dir));

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_mkdir_recursive() {
    std::cout << "  test_mkdir_recursive... ";
    auto testDir = createTestDir();

    std::string deep = testDir + "/a/b/c/d";
    DirectoryService::createDirectory(deep, true);
    assert(PathUtils::isDirectory(deep));

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

void test_rmdir_notEmpty() {
    std::cout << "  test_rmdir_notEmpty... ";
    auto testDir = createTestDir();

    std::string dir = testDir + "/notempty";
    DirectoryService::createDirectory(dir);
    FileService::createFile(dir + "/file.txt");

    try {
        DirectoryService::removeDirectory(dir);
        assert(false && "Should have thrown");
    } catch (const DirectoryNotEmptyException&) {
        // expected
    }

    cleanupTestDir(testDir);
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== FileService Tests ===\n";

    test_createFile();
    test_readWriteFile();
    test_appendFile();
    test_deleteFile();
    test_deleteFile_notFound();
    test_copyFile();
    test_copyFile_exists();
    test_moveFile();
    test_emptyFile();
    test_mkdir_and_rmdir();
    test_mkdir_recursive();
    test_rmdir_notEmpty();

    std::cout << "\nAll FileService tests PASSED!\n";
    return 0;
}
