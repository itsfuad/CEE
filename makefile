CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cee
SOURCES = main.c
HEADERS = memtool.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
    $(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
    rm -f $(TARGET)