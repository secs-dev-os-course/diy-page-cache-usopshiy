#pragma once

int generate_test_file(const char *path);

int test_without_cache(const char *path);

int test_with_custom_cache(const char *path);

int test_with_cache(const char *path);