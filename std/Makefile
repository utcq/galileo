
SOURCES := $(shell find . -name '*.c')
OUTPUT := stdlib.so
CFLAGS := -std=gnu23 -g -fPIC -shared

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@