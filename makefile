# Determine platform
ifeq ($(OS),Windows_NT)
	PLATFORM = windows
	CC = x86_64-w64-mingw32-gcc
	CFLAGS = -Wall -Wextra -O2
	LDFLAGS = -lpsapi
	LIBS = -lpsapi
	EXEC = cee.exe
	TEST_EXEC = test_cee.exe
else
	PLATFORM = linux
	CC = gcc
	CFLAGS = -Wall -Wextra -O2
	LDFLAGS =
	LIBS =
	EXEC = cee
	TEST_EXEC = test_cee
endif

# Sources
SOURCES = main.c memtool.c
TEST_SOURCES = test_main.c memtool.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Build targets
all: $(EXEC) $(TEST_EXEC)

# Main executable
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LIBS)

# Test executable (without including main.c)
$(TEST_EXEC): $(TEST_OBJECTS)
	$(CC) $(TEST_OBJECTS) -o $(TEST_EXEC) $(LIBS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(EXEC) $(TEST_OBJECTS) $(TEST_EXEC)
