#include "tests.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <lab2/lab2.hpp>

#define PAGE_SIZE 4096
#define TEST_FILE_SIZE (PAGE_SIZE * 1024)

int generate_test_file(const char *path) {
    std::srand(std::time({}));
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if(fd < 0) {
        return -1;
    }

    char *buf = (char *) malloc(PAGE_SIZE);
    memset(buf, 'S', PAGE_SIZE);

    for (size_t i = 0; i < TEST_FILE_SIZE; i += PAGE_SIZE) {
        if (write(fd, buf, PAGE_SIZE) != PAGE_SIZE) {
            return(-1);
        }
    }

    free(buf);
    close(fd);

    return 0;
}

int test_without_cache(const char *path) {
    int fd = open(path, O_RDONLY | O_DIRECT);

    if (fd == -1) {
        return -1;
    }

    char *buf;
    posix_memalign((void **)&buf, PAGE_SIZE, PAGE_SIZE);

    clock_t start = clock();
    for (size_t i = 0; i < 10000; i++) {
        int j = (std::rand()) % (TEST_FILE_SIZE / PAGE_SIZE);
        pread(fd, buf, PAGE_SIZE, j * PAGE_SIZE);
    }
    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time without cache: %.6f seconds\n", elapsed);

    free(buf);
    close(fd);

    return 0;
}

int test_with_custom_cache(const char *path) {
    int fd = lab2_open(path);

    if (fd == -1) {
        return -1;
    }

    char *buf = (char *)malloc(PAGE_SIZE);

    clock_t start = clock();
    for (size_t i = 0; i < 10000; i++) {
        int j = (std::rand()) % (TEST_FILE_SIZE / PAGE_SIZE);
        lab2_lseek(fd, j * PAGE_SIZE, SEEK_SET);
        lab2_read(fd, buf, PAGE_SIZE);
    }
    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time with custom cache: %.6f seconds\n", elapsed);

    free(buf);
    lab2_close(fd);

    return 0;
}

int test_with_cache(const char *path) {
    int fd = open(path, O_RDWR);

    if (fd == -1) {
        return -1;
    }

    char *buf = (char *)malloc(PAGE_SIZE);

    clock_t start = clock();
    for (size_t i = 0; i < 10000; i++) {
        int j = (i * 435454321) % (TEST_FILE_SIZE / PAGE_SIZE);
        pread(fd, buf, PAGE_SIZE, j * PAGE_SIZE);
    }
    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time with linux cache: %.6f seconds\n", elapsed);

    free(buf);
    close(fd);

    return 0;
}