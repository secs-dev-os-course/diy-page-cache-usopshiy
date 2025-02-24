#include <cstdio>
#include <lab2/lab2.hpp>
#include "tests.hpp"

#define TEST_FILE_PATH "testfile"

int main() {
    if (generate_test_file(TEST_FILE_PATH) == -1) {
        perror("Failed to generate test file, evicting");
        return -1;
    }

    if (test_without_cache(TEST_FILE_PATH) == -1) {
        perror("Failed to run test without cache");
    }

    if (test_with_custom_cache(TEST_FILE_PATH) == -1) {
        perror("Failed to run test with custom cache");
    }

    test_with_cache(TEST_FILE_PATH);
    return 0;
}
