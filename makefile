# Determine platform
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    CC = x86_64-w64-mingw32-gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -lpsapi
    LIBS = -lpsapi
    SOURCES = main.c memtool.c
    TEST_SOURCES = test_main.c
    OBJECTS = $(SOURCES:.c=.o)
    TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
    EXEC = cee.exe
    TEST_EXEC = test_cee.exe
else
    PLATFORM = linux
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS =
    LIBS =
    SOURCES = main.c memtool.c
    TEST_SOURCES = test_main.c
    OBJECTS = $(SOURCES:.c=.o)
    TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
    EXEC = cee
    TEST_EXEC = test_cee
endif

all: $(EXEC) $(TEST_EXEC)

$(EXEC): $(OBJECTS)
    $(CC) $(OBJECTS) -o $(EXEC) $(LIBS)

$(TEST_EXEC): $(TEST_OBJECTS) $(OBJECTS)
    $(CC) $(TEST_OBJECTS) $(OBJECTS) -o $(TEST_EXEC) $(LIBS)

clean:
    rm -f $(OBJECTS) $(EXEC) $(TEST_OBJECTS) $(TEST_EXEC)