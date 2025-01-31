#include "memtool.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_open_close_process(process_id_t pid) {
    ProcessHandle* handle = open_process(pid);
    assert(handle != NULL && "Failed to open process");
    close_process(handle);
    printf("test_open_close_process passed.\n");
}

void test_read_process_maps(process_id_t pid) {
    ProcessMap* map = read_process_maps(pid);
    assert(map != NULL && "Failed to read process memory map");
    assert(map->count > 0 && "Process memory map should not be empty");
    print_memory_map(map);
    free(map);
    printf("test_read_process_maps passed.\n");
}

void test_read_write_memory(process_id_t pid) {
    ProcessHandle* handle = open_process(pid);
    assert(handle != NULL && "Failed to open process");

    char test_data[] = "Hello";
    char buffer[sizeof(test_data)] = {0};

    // Assuming a writable address is known
    void* test_address = (void*)0x12345678; 

    size_t written = write_process_memory(handle, test_address, test_data, sizeof(test_data));
    assert(written == sizeof(test_data) && "Failed to write memory");

    size_t read = read_process_memory(handle, test_address, buffer, sizeof(buffer));
    assert(read == sizeof(test_data) && "Failed to read memory");
    assert(memcmp(test_data, buffer, sizeof(test_data)) == 0 && "Memory data mismatch");

    close_process(handle);
    printf("test_read_write_memory passed.\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    process_id_t pid = atoi(argv[1]);
    test_open_close_process(pid);
    test_read_process_maps(pid);
    // test_read_write_memory(pid); // Uncomment if a writable memory address is known

    printf("All tests completed successfully.\n");
    return 0;
}
