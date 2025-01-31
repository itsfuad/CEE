// memtool.h
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <psapi.h>
    #include <tlhelp32.h>
    typedef DWORD process_id_t;
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #define PLATFORM_UNIX
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    typedef pid_t process_id_t;
#else
    #error "Unsupported platform"
#endif

#define BUFFER_SIZE 4096
#define MAX_REGIONS 1024
#define MAX_PATH_LEN 512

// Cross-platform memory region structure
typedef struct {
    #ifdef PLATFORM_WINDOWS
        PVOID BaseAddress;
        SIZE_T RegionSize;
        DWORD Protection;
    #else
        unsigned long start;
        unsigned long end;
        unsigned long size;
        char perms[5];
    #endif
    char pathname[MAX_PATH_LEN];
} MemoryRegion;

typedef struct {
    MemoryRegion regions[MAX_REGIONS];
    int count;
} ProcessMap;

// Cross-platform process handle
typedef struct {
    #ifdef PLATFORM_WINDOWS
        HANDLE handle;
    #else
        int pid;
        int mem_fd;
    #endif
} ProcessHandle;

// Function prototypes
ProcessHandle* open_process(process_id_t pid);
void close_process(ProcessHandle* handle);
ProcessMap* read_process_maps(process_id_t pid);
size_t read_process_memory(ProcessHandle* handle, void* address, void* buffer, size_t size);
size_t write_process_memory(ProcessHandle* handle, void* address, void* buffer, size_t size);
void search_pattern(ProcessHandle* handle, const MemoryRegion* region, const char* pattern, size_t pattern_len);
void print_memory_map(const ProcessMap* map);