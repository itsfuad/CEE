#include "memtool.h"


/**
 * @file main.c
 * @brief Contains the main function which serves as the program's entry point.
 *
 * The main function accepts command-line arguments and returns an integer 
 * status code to indicate the success or failure of program execution.
 *
 * @param argc The number of command-line arguments.
 * @param argv Array of command-line arguments, represented as strings.
 *
 * @return int Returns 0 if the program executes successfully, non-zero otherwise.
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <command> <pid> [options]\n", argv[0]);
        printf("Commands: map, search, write\n");
        return 1;
    }

    // Get the process ID from the command-line arguments
    process_id_t pid = atoi(argv[2]);
    ProcessHandle* handle = open_process(pid);
    if (!handle) {
        printf("Failed to open process %d\n", pid);
        return 1;
    }

    if (strcmp(argv[1], "map") == 0) {
        ProcessMap* map = read_process_maps(pid);
        if (map) {
            print_memory_map(map);
            free(map);
        }
    }
    else if (strcmp(argv[1], "search") == 0) {
        if (argc < 4) {
            printf("Usage: %s search <pid> <pattern>\n", argv[0]);
            close_process(handle);
            return 1;
        }

        ProcessMap* map = read_process_maps(pid);
        if (map) {
            for (int i = 0; i < map->count; i++) {
                // Only search readable memory
                #ifdef PLATFORM_WINDOWS
                if (map->regions[i].Protection & PAGE_READONLY)
                #else
                if (map->regions[i].perms[0] == 'r')
                #endif
                {
                    search_pattern(handle, &map->regions[i],
                                 argv[3], strlen(argv[3]));
                }
            }
            free(map);
        }
    }
    else if (strcmp(argv[1], "write") == 0) {
        if (argc < 5) {
            printf("Usage: %s write <pid> <address> <data>\n", argv[0]);
            close_process(handle);
            return 1;
        }

        void* address = (void*)strtoul(argv[3], NULL, 0);
        char* data = argv[4];
        size_t data_len = strlen(data);

        size_t bytes_written = write_process_memory(handle, address, data, data_len);
        if (bytes_written == data_len) {
            printf("Successfully wrote %zu bytes to address %p\n", bytes_written, address);
        } else {
            printf("Failed to write to address %p\n", address);
        }
    }

    close_process(handle);
    return 0;
}