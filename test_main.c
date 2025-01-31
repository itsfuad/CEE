#include <assert.h>
#include <stdio.h>
#include "memtool.h"

#include "memtool.h"

process_id_t get_process_id() {
    #ifdef PLATFORM_WINDOWS
        return GetCurrentProcessId();
    #else
        return getpid();
    #endif
}


void test_open_process() {
    printf("Testing open_process...\n");
    //grab own process id
    process_id_t pid = get_process_id();
    printf("PID: %d\n", pid);
    ProcessHandle* handle = open_process(pid);
    printf("Handle: %p\n", handle);
    assert(handle != NULL);
    close_process(handle);
}

void test_read_process_maps() {
    printf("Testing read_process_maps...\n");
    process_id_t pid = get_process_id();
    printf("PID: %d\n", pid);
    ProcessMap* map = read_process_maps(pid);
    printf("Map: %p\n", map);
    assert(map != NULL);
    free(map);
}

int main() {
    test_open_process();
    test_read_process_maps();
    printf("All tests passed!\n");
    return 0;
}