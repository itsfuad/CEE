# CEE

CEE is a cross-platform utility for reading and searching process memory. It supports both Windows and Unix-like systems.

## Features
- Open and close processes
- Read process memory maps
- Search for patterns in process memory

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
Building
To build the project, you need to have gcc installed. Use the provided Makefile:
```sh
make
```