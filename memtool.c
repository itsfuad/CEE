// main.c
#include "memtool.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <winnt.h>
#include <psapi.h>
#endif

// Platform-specific process open
/**
 * @brief Opens a process given its process ID.
 *
 * This function attempts to open a process associated with the specified process ID.
 * It returns a pointer to a ProcessHandle structure if the process is successfully opened;
 * otherwise, it returns NULL indicating failure.
 *
 * @param pid The unique identifier (process_id_t) of the target process.
 *
 * @return Pointer to the process handle on success, or NULL on failure.
 */
ProcessHandle* open_process(process_id_t pid) {
    ProcessHandle* handle = malloc(sizeof(ProcessHandle));
    if (!handle) {
        perror("Failed to allocate memory for ProcessHandle");
        return NULL;
    }

    #ifdef PLATFORM_WINDOWS // Windows-specific process open
        handle->handle = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
            FALSE,
            pid
        );
        if (handle->handle == NULL) {
            perror("Failed to open process");
            free(handle);
            return NULL;
        }
    #else // Linux-specific process open
        handle->pid = pid;
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/mem", pid);
        handle->mem_fd = open(path, O_RDWR);
        if (handle->mem_fd == -1) {
            perror("Failed to open process memory file");
            free(handle);
            return NULL;
        }
    #endif

    return handle;
}

// Platform-specific process close
/**
 * @brief Closes the process associated with the provided process handle.
 *
 * This function releases any resources tied to the process and performs
 * necessary cleanup. It should be called when the process is no longer needed.
 *
 * @param handle Pointer to a ProcessHandle structure representing the process to be closed.
 */
void close_process(ProcessHandle* handle) {
    if (!handle) return;

    #ifdef PLATFORM_WINDOWS
        if (handle->handle) CloseHandle(handle->handle);
    #else
        if (handle->mem_fd != -1) close(handle->mem_fd);
    #endif

    free(handle);
}

// Read process memory maps
/**
 * @brief Reads the memory maps for the specified process.
 *
 * This function retrieves the memory layout information for a process
 * identified by the given process id. It returns a pointer to a ProcessMap
 * structure containing the memory map details.
 *
 * @param pid The unique identifier of the process whose memory maps are to be read.
 * @return Pointer to a ProcessMap structure containing the process memory maps,
 *         or NULL if the operation fails.
 */
ProcessMap* read_process_maps(process_id_t pid) {
    // Allocate memory for the process map
    ProcessMap* map = malloc(sizeof(ProcessMap));
    if (!map) {
        perror("Failed to allocate memory for ProcessMap");
        return NULL;
    }
    map->count = 0;

    #ifdef PLATFORM_WINDOWS
        // Open the target process
        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            pid
        );
        if (hProcess == NULL) {
            perror("Failed to open process");
            free(map);
            return NULL;
        }

        MEMORY_BASIC_INFORMATION mbi;
        PVOID address = 0;

        // Enumerate memory regions
        while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (map->count >= MAX_REGIONS) break;

            MemoryRegion* region = &map->regions[map->count];
            region->BaseAddress = mbi.BaseAddress;
            region->RegionSize = mbi.RegionSize;
            region->Protection = mbi.Protect;

            // Get module name if available
            if (GetModuleFileNameExA(hProcess, NULL, 
                region->pathname, MAX_PATH_LEN) == 0) {
                region->pathname[0] = '\0';
            }

            map->count++;
            address = (PVOID)((DWORD_PTR)address + mbi.RegionSize);
        }

        CloseHandle(hProcess);
    #else
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/maps", pid);
        FILE* maps = fopen(path, "r");
        if (!maps) {
            perror("Failed to open process maps file");
            free(map);
            return NULL;
        }

        char line[1024];
        while (fgets(line, sizeof(line), maps) && map->count < MAX_REGIONS) {
            MemoryRegion* region = &map->regions[map->count];
            
            // Parse maps line
            unsigned long start, end;
            char perms[5], pathname[MAX_PATH_LEN];
            if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %255s",
                      &start, &end, perms, pathname) >= 3) {
                region->start = start;
                region->end = end;
                region->size = end - start;
                strncpy(region->perms, perms, sizeof(region->perms));
                strncpy(region->pathname, pathname, sizeof(region->pathname));
                map->count++;
            }
        }

        fclose(maps);
    #endif

    return map;
}

// Read process memory
size_t read_process_memory(ProcessHandle* handle, void* address, 
                          void* buffer, size_t size) {
    #ifdef PLATFORM_WINDOWS
        SIZE_T bytes_read = 0;
        if (ReadProcessMemory(handle->handle, address, buffer, size, &bytes_read)) {
            return bytes_read;
        }
        return 0;
    #else
        ssize_t bytes_read = pread(handle->mem_fd, buffer, size, (off_t)address);
        return bytes_read > 0 ? bytes_read : 0;
    #endif
}

// Write process memory
size_t write_process_memory(ProcessHandle* handle, void* address, 
                           void* buffer, size_t size) {
    #ifdef PLATFORM_WINDOWS
        SIZE_T bytes_written = 0;
        if (WriteProcessMemory(handle->handle, address, buffer, size, &bytes_written)) {
            return bytes_written;
        }
        return 0;
    #else
        ssize_t bytes_written = pwrite(handle->mem_fd, buffer, size, (off_t)address);
        return bytes_written > 0 ? bytes_written : 0;
    #endif
}

// Search for pattern in memory region
/**
 * @brief Searches for a specific memory pattern within a given memory region.
 *
 * This function performs a scan of the memory region specified by @c region, using the
 * process handle provided by @c handle to access the target process's memory. The search
 * logic, which is implemented within this function, identifies occurrences of a particular
 * pattern based on predetermined criteria.
 *
 * @param handle A pointer to the process's handle, used for accessing and manipulating
 *               the process's memory.
 * @param region A pointer to a MemoryRegion structure that describes the memory area to be scanned.
 */
void search_pattern(ProcessHandle* handle, const MemoryRegion* region,
                   const char* pattern, size_t pattern_len) {
    char* buffer = malloc(BUFFER_SIZE);
    if (!buffer) return;

    #ifdef PLATFORM_WINDOWS
        SIZE_T offset = 0;
        while (offset < region->RegionSize) {
            size_t bytes_read = read_process_memory(handle,
                (char*)region->BaseAddress + offset,
                buffer, BUFFER_SIZE);
    #else
        size_t offset = 0;
        while (offset < region->size) {
            size_t bytes_read = read_process_memory(handle,
                (void*)(region->start + offset),
                buffer, BUFFER_SIZE);
    #endif

            if (bytes_read == 0) break;

            for (size_t i = 0; i < bytes_read; i++) {
                if (i + pattern_len > bytes_read) break;
                
                if (memcmp(buffer + i, pattern, pattern_len) == 0) {
                    #ifdef PLATFORM_WINDOWS
                        printf("Pattern found at: %p\n",
                            (void*)((char*)region->BaseAddress + offset + i));
                    #else
                        printf("Pattern found at: 0x%lx\n",
                            (unsigned long)(region->start + offset + i));
                    #endif
                }
            }

            offset += bytes_read;
        }

    free(buffer);
}

/**
 * print_memory_map - Prints the memory layout of a process.
 *
 * This function takes a pointer to a ProcessMap structure, which contains the memory
 * mapping information for a process, and outputs details that help visualize the
 * process's memory segments. The details typically include segment starting addresses,
 * sizes, and possibly permissions or other attributes.
 *
 * Parameters:
 *   @map: A constant pointer to a ProcessMap structure representing the process's memory map.
 *
 * Returns:
 *   void.
 *
 * Note:
 *   Ensure that the provided ProcessMap pointer is not NULL before invoking this function.
 */
void print_memory_map(const ProcessMap* map) {
    printf("Memory regions:\n");
    for (int i = 0; i < map->count; i++) {
        #ifdef PLATFORM_WINDOWS
            printf("%p - %p ",
                   map->regions[i].BaseAddress,
                   (char*)map->regions[i].BaseAddress + map->regions[i].RegionSize);
            
            if (map->regions[i].Protection & PAGE_EXECUTE) printf("X");
            if (map->regions[i].Protection & PAGE_READONLY) printf("R");
            if (map->regions[i].Protection & PAGE_READWRITE) printf("W");
        #else
            printf("0x%lx - 0x%lx %s",
                   map->regions[i].start,
                   map->regions[i].end,
                   map->regions[i].perms);
        #endif

        if (map->regions[i].pathname[0])
            printf(" %s", map->regions[i].pathname);
        printf("\n");
    }
}
