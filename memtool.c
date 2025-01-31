// main.c
#include "memtool.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <winnt.h>
#include <psapi.h>
#endif

// Platform-specific process open
ProcessHandle* open_process(process_id_t pid) {
    ProcessHandle* handle = malloc(sizeof(ProcessHandle));
    if (!handle) return NULL;

    #ifdef PLATFORM_WINDOWS
        handle->handle = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
            FALSE,
            pid
        );
        if (handle->handle == NULL) {
            free(handle);
            return NULL;
        }
    #else
        handle->pid = pid;
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/mem", pid);
        handle->mem_fd = open(path, O_RDWR);
        if (handle->mem_fd == -1) {
            free(handle);
            return NULL;
        }
    #endif

    return handle;
}

// Platform-specific process close
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
ProcessMap* read_process_maps(process_id_t pid) {
    ProcessMap* map = malloc(sizeof(ProcessMap));
    if (!map) return NULL;
    map->count = 0;

    #ifdef PLATFORM_WINDOWS
        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE,
            pid
        );
        if (hProcess == NULL) {
            free(map);
            return NULL;
        }

        MEMORY_BASIC_INFORMATION mbi;
        PVOID address = 0;

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
