# compile cee
CC = gcc
# -Wall: show all warnings
# -Wextra: show extra warnings
# -O2: optimize code
CFLAGS = -Wall -Wextra -O2
# Target file name
TARGET = cee
# Source files
SOURCES = main.c memtool.c
# Header files
HEADERS = memtool.h
# libs
LIBS = -lpsapi

# Makefile rules. all means that the target is the default target
all: $(TARGET)
$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)  
