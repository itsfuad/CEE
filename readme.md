# CEE

CEE is a cross-platform utility for reading and searching process memory. It supports both Windows and Unix-like systems.

## Features
- Open and close processes
- Read process memory maps
- Search for patterns in process memory
- Write to process memory

## Prerequisites
- GCC or MinGW for compilation
- Make utility

## Setup
Clone the repository:
```sh
git clone https://github.com/yourusername/cee.git
cd cee
```

## Building
To build the project, use the provided Makefile:
```sh
make
```

## Usage
```sh
./cee <command> <pid> [options]
```

### Examples
Print memory map of process with PID 1234:
```sh
./cee map 1234
```

Search for the pattern "example" in the memory of process with PID 1234:
```sh
./cee search 1234 example
```

Write data to the memory of process with PID 1234 at address 0x7ffdf000:
```sh
./cee write 1234 0x7ffdf000 "data"
```

## License
See the [LICENSE](LICENSE) file for license rights and limitations (GNU GPLv3).

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change. Make sure your code passes the workflow checks.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request

## Attribution
Cover image from [GeeksforGeeks](https://www.geeksforgeeks.org/memory-inspector-tool-in-microsoft-edge-browser/)