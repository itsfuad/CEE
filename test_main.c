#include <assert.h>
#include <stdio.h>
#include "memtool.h"

void test_open_process() {
    process_id_t pid = 1; // Example PID
    ProcessHandle* handle = open_process(pid);
    assert(handle != NULL);
    close_process(handle);
}

void test_read_process_maps() {
    process_id_t pid = 1; // Example PID
    ProcessMap* map = read_process_maps(pid);
    assert(map != NULL);
    free(map);
}

int main() {
    test_open_process();
    test_read_process_maps();
    printf("All tests passed!\n");
    return 0;
}