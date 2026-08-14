///////////////////////////////////////////////////////////////////////////////
// test_string_utils.cpp — Unit tests for StringUtils
///////////////////////////////////////////////////////////////////////////////

#include "utils/StringUtils.hpp"

#include <cassert>
#include <iostream>
#include <sys/stat.h>

using namespace filemgr;

void test_humanReadableSize() {
    std::cout << "  test_humanReadableSize... ";

    assert(StringUtils::humanReadableSize(0) == "0 B");
    assert(StringUtils::humanReadableSize(512) == "512 B");
    assert(StringUtils::humanReadableSize(1024) == "1.0 KB");
    assert(StringUtils::humanReadableSize(1536) == "1.5 KB");
    assert(StringUtils::humanReadableSize(1048576) == "1.0 MB");
    assert(StringUtils::humanReadableSize(1073741824) == "1.0 GB");

    std::cout << "PASSED\n";
}

void test_globToRegex() {
    std::cout << "  test_globToRegex... ";

    assert(StringUtils::globToRegex("*.cpp") == "^.*\\.cpp$");
    assert(StringUtils::globToRegex("file?.txt") == "^file.\\.txt$");
    assert(StringUtils::globToRegex("*") == "^.*$");
    assert(StringUtils::globToRegex("exact") == "^exact$");

    std::cout << "PASSED\n";
}

void test_formatPermissions() {
    std::cout << "  test_formatPermissions... ";

    // 0755 = rwxr-xr-x
    mode_t mode755 = S_IRUSR | S_IWUSR | S_IXUSR |
                     S_IRGRP | S_IXGRP |
                     S_IROTH | S_IXOTH;
    assert(StringUtils::formatPermissions(mode755) == "rwxr-xr-x");

    // 0644 = rw-r--r--
    mode_t mode644 = S_IRUSR | S_IWUSR |
                     S_IRGRP |
                     S_IROTH;
    assert(StringUtils::formatPermissions(mode644) == "rw-r--r--");

    // 0600 = rw-------
    mode_t mode600 = S_IRUSR | S_IWUSR;
    assert(StringUtils::formatPermissions(mode600) == "rw-------");

    std::cout << "PASSED\n";
}

void test_formatOctalPermissions() {
    std::cout << "  test_formatOctalPermissions... ";

    mode_t mode755 = S_IRUSR | S_IWUSR | S_IXUSR |
                     S_IRGRP | S_IXGRP |
                     S_IROTH | S_IXOTH;
    assert(StringUtils::formatOctalPermissions(mode755) == "0755");

    std::cout << "PASSED\n";
}

void test_toLower() {
    std::cout << "  test_toLower... ";

    assert(StringUtils::toLower("Hello World") == "hello world");
    assert(StringUtils::toLower("ALLCAPS") == "allcaps");
    assert(StringUtils::toLower("already") == "already");
    assert(StringUtils::toLower("") == "");

    std::cout << "PASSED\n";
}

void test_trim() {
    std::cout << "  test_trim... ";

    assert(StringUtils::trim("  hello  ") == "hello");
    assert(StringUtils::trim("\t\nhello\n\t") == "hello");
    assert(StringUtils::trim("no_whitespace") == "no_whitespace");
    assert(StringUtils::trim("   ") == "");
    assert(StringUtils::trim("") == "");

    std::cout << "PASSED\n";
}

void test_split() {
    std::cout << "  test_split... ";

    auto parts = StringUtils::split("a/b/c", '/');
    assert(parts.size() == 3);
    assert(parts[0] == "a");
    assert(parts[1] == "b");
    assert(parts[2] == "c");

    parts = StringUtils::split("single", '/');
    assert(parts.size() == 1);
    assert(parts[0] == "single");

    std::cout << "PASSED\n";
}

void test_startsWith_endsWith() {
    std::cout << "  test_startsWith_endsWith... ";

    assert(StringUtils::startsWith("hello world", "hello") == true);
    assert(StringUtils::startsWith("hello world", "world") == false);
    assert(StringUtils::endsWith("hello world", "world") == true);
    assert(StringUtils::endsWith("hello world", "hello") == false);

    std::cout << "PASSED\n";
}

void test_padLeft_padRight() {
    std::cout << "  test_padLeft_padRight... ";

    assert(StringUtils::padLeft("42", 5) == "   42");
    assert(StringUtils::padLeft("42", 5, '0') == "00042");
    assert(StringUtils::padRight("hi", 5) == "hi   ");
    assert(StringUtils::padLeft("toolong", 3) == "toolong"); // no truncation

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== StringUtils Tests ===\n";

    test_humanReadableSize();
    test_globToRegex();
    test_formatPermissions();
    test_formatOctalPermissions();
    test_toLower();
    test_trim();
    test_split();
    test_startsWith_endsWith();
    test_padLeft_padRight();

    std::cout << "\nAll StringUtils tests PASSED!\n";
    return 0;
}
