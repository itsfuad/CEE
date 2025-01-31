# CEE

CEE is a cross-platform utility for reading and searching process memory. It supports both Windows and Unix-like systems.

## Features
- Open and close processes
- Read process memory maps
- Search for patterns in process memory
- Write to process memory

## Usage
```sh
./cee <command> <pid> [options]
```

Examples
Print memory map of process with PID 1234:

```sh
./cee map 1234
```

Search for the pattern "example" in the memory of process with PID 1234:

```sh
./cee search 1234 example
```

Write data to the memory of process with PID 1234 at address 0x7ffdf000:
But be careful, writing to memory can cause the process to crash or behave unexpectedly.
```sh
./cee write 1234 0x7ffdf000 "data"
```

Building
To build the project, you need to have gcc installed. Use the provided Makefile:
```sh
make
```

## License
See the [LICENSE](LICENSE) file for license rights and limitations (MIT).
```

## Attribution
Cover image from [GeeksforGeeks](https://www.geeksforgeeks.org/memory-inspector-tool-in-microsoft-edge-browser/)

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change. Make sure your code passes the workflow checks.