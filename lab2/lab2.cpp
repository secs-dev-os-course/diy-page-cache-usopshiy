#include "lab2.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>

#define PAGE_AMOUNT 1024 // to compare with normal - wnat to put all data in cache
#define PAGE_SIZE 4096

typedef struct {
    off_t offset; // offset in file
    int reference; // reference for clock
    int dirty; // modification flag
    void *data;
} cache_page_t;

typedef struct {
    cache_page_t pages[PAGE_AMOUNT];
    size_t clock_hand; //pointer for clock algo
    int fd;
} cache_t;

static cache_t cache = {
    .clock_hand = 0,
    .fd = -1
};

// intializing cache: setup structure and put in memory
static int cache_init() {
    for (size_t i = 0; i < PAGE_AMOUNT; i++) {
        posix_memalign(&cache.pages[i].data, PAGE_SIZE, PAGE_SIZE); //memory allocation
        cache.pages[i].offset = -1; //page not linked yet
        cache.pages[i].reference = 0; //page is free to write
        cache.pages[i].dirty = 0; 
    }
    return cache.fd;
}

static cache_page_t* cache_lookup(off_t offset) {
    for (size_t i = 0; i < PAGE_AMOUNT; i++) {
        if (cache.pages[i].offset == offset) {
            return &cache.pages[i];
        }
    }

    return NULL;
}

static cache_page_t* cache_evict() {
    while(1) {
        cache_page_t *page = &cache.pages[cache.clock_hand];
        if (page->reference == 0) {
            cache.clock_hand = (cache.clock_hand + 1) % PAGE_AMOUNT;
            return page;
        } 
        else {
            page->reference = 0;
            cache.clock_hand = (cache.clock_hand + 1) % PAGE_AMOUNT;
        }
    }
}

static int sync_page(cache_page_t *page) {
    if (page->dirty && page->offset != -1) {
        if (pwrite(cache.fd, page->data, PAGE_SIZE, page->offset) != PAGE_SIZE) {
            return -1;
        }
        page->dirty = 0;
    }
    
    return 0;
}

static int load_page(cache_page_t *page, off_t offset) {
    sync_page(page); // make sure to save page if it is evicted
    ssize_t bytes_read = pread(cache.fd, page->data, PAGE_SIZE, offset);
    if (bytes_read == -1) {
        return - 1;
    }
    page->offset = offset;
    page->dirty = 0;
    page->reference = 1;
    return bytes_read;
}

int lab2_open(const char *path) {
    cache.fd = open(path, O_RDWR | O_DIRECT);
    if (cache.fd == -1) {
        return -1;
    }
    return cache_init();
}

int lab2_close(int fd) {
    if (fd != cache.fd) {
        return -1;
    }

    for (size_t i = 0; i < PAGE_AMOUNT; i++) {
        sync_page(&cache.pages[i]);
        free(cache.pages[i].data);
    }

    cache.fd = -1;
    return close(fd);
}

ssize_t lab2_read(int fd, void *buf, size_t count) {
    if (fd != cache.fd) {
        return -1;
    }

    size_t total_read = 0;
    size_t pos = lseek(fd, 0, SEEK_CUR);
    while (count > 0) {
        size_t page_offset = pos % PAGE_SIZE;
        off_t file_offset = (pos / PAGE_SIZE) * PAGE_SIZE;
        size_t to_copy = PAGE_SIZE - page_offset;
        if (to_copy > count) {
            to_copy = count;
        }

        cache_page_t *page = cache_lookup(file_offset);
        if (!page) {
            page = cache_evict();
            if (load_page(page, file_offset) == -1) {
                return -1;
            }
        }
        
        memcpy((char *) buf, (char *)page->data + page_offset, to_copy);

        buf = (char *) buf + to_copy;
        count -= to_copy;
        total_read += to_copy;
        pos += to_copy;
    }

    return total_read;
}

ssize_t lab2_write(int fd, const void *buf, size_t count) {
    if (fd != cache.fd) {
        return -1;
    }

    size_t total_written = 0;
    size_t page_offset = lseek(fd, 0, SEEK_CUR) % PAGE_SIZE;

    while (count > 0) {
        off_t file_offset = (lseek(fd, 0, SEEK_CUR) / PAGE_SIZE) * PAGE_SIZE;
        size_t to_copy = PAGE_SIZE - page_offset;
        if (to_copy > count) {
            to_copy = count;
        }
        
        cache_page_t *page = cache_lookup(file_offset);
        if (!page) {
            page = cache_evict();
            if (load_page(page, file_offset) == -1) {
                return -1;
            }
        }

        memcpy((char *)page->data + page_offset, buf, to_copy);
        page->dirty = 1;

        buf = (const char *)buf + to_copy;
        count -= to_copy;
        total_written += to_copy;
        page_offset = 0;
        lseek(fd, to_copy, SEEK_CUR);
    }

    return total_written;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
    if (fd != cache.fd) {
        return -1;
    }

    return lseek(fd, offset, whence);
}

int lab2_fsync(int fd) {
    if (fd != cache.fd) {
        return -1;
    }

    for (size_t i = 0; i < PAGE_AMOUNT; i++) {
        if (sync_page(&cache.pages[i]) == -1) {
            return -1;
        }
    }

    return fsync(fd);
}