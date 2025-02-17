/**
 * @file memtool.h
 * @brief Cross-platform library for process memory inspection and manipulation.
 *
 * This header defines a set of functions and data structures that allow users 
 * to open a process, read and write its memory, enumerate memory regions, and 
 * search for patterns within those regions.
 *
 * The library supports both Windows and Unix-like operating systems by 
 * conditionally including the appropriate headers and defining platform-specific 
 * types and structures.
 *
 * @note The code leverages platform detection macros (PLATFORM_WINDOWS and PLATFORM_UNIX)
 * to ensure proper functionality across systems.
 *
 * @def BUFFER_SIZE
 * A constant specifying the generic buffer size (4096 bytes).
 *
 * @def MAX_REGIONS
 * The maximum number of memory regions that can be stored in a ProcessMap.
 *
 * @def MAX_PATH_LEN
 * The maximum length allotted for storing a pathname.
 *
 * @struct MemoryRegion
 * @brief Represents a memory region in a process.
 *
 * For Windows, the structure includes:
 *  - BaseAddress: Pointer to the beginning of the memory region.
 *  - RegionSize: Size of the memory region.
 *  - Protection: Memory protection flags.
 *
 * For Unix-like systems, the structure includes:
 *  - start: Start address of the memory region.
 *  - end: End address of the memory region.
 *  - size: Overall size of the memory region.
 *  - perms: Permissions string (read, write, execute, etc.).
 *
 * In addition, both versions include a pathname for the memory region.
 *
 * @struct ProcessMap
 * @brief Contains an array of MemoryRegion structures describing the process's
 * memory layout and a count of total regions.
 *
 * @struct ProcessHandle
 * @brief Abstracts a process handle.
 *
 * On Windows, it wraps the HANDLE type.
 * On Unix-like systems, it includes the process ID (pid) and file descriptor (mem_fd) to memory.
 *
 * @function open_process
 * @brief Opens a process given its process identifier.
 *
 * @param pid Process identifier.
 * @return Pointer to a new ProcessHandle if successful, or NULL on failure.
 *
 * @function close_process
 * @brief Closes the given process handle and releases associated resources.
 *
 * @param handle ProcessHandle to be closed.
 *
 * @function read_process_maps
 * @brief Retrieves the memory map (list of memory regions) for a process.
 *
 * @param pid Process identifier.
 * @return Pointer to a ProcessMap structure populated with memory regions.
 *
 * @function read_process_memory
 * @brief Reads a block of memory from a specified address in the process.
 *
 * @param handle Process handle.
 * @param address Starting address from where to read.
 * @param buffer Buffer to store the read data.
 * @param size Number of bytes to read.
 * @return Number of bytes successfully read.
 *
 * @function write_process_memory
 * @brief Writes a block of data to a specified address in the process.
 *
 * @param handle Process handle.
 * @param address Destination address in the process.
 * @param buffer Data buffer containing the data to write.
 * @param size Number of bytes to write.
 * @return Number of bytes successfully written.
 *
 * @function search_pattern
 * @brief Searches for a given pattern within a specified memory region.
 *
 * @param handle Process handle.
 * @param region Memory region within which to search.
 * @param pattern Byte pattern to locate.
 * @param pattern_len Length of the pattern in bytes.
 *
 * @function print_memory_map
 * @brief Prints a human-readable representation of the process's memory map.
 *
 * @param map Pointer to a ProcessMap structure.
 *
 * Overall, this library facilitates low-level process memory operations and is
 * particularly useful for tasks such as debugging, reverse engineering, or system introspection.
 */
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