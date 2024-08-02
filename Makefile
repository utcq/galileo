
SRC_DIR := src/
INCLUDES := -I$(SRC_DIR)
LIBS :=
TEST_FLAGS := -Wl,--defsym=main=__test_main -Wl,-wrap,exit -Wl,-wrap,abort -Wl,-wrap,assert
CFLAGS := -Wall -Wextra -Werror -std=gnu23 -g $(INCLUDES) $(LIBS) $(TEST_FLAGS)

CC := clang
ifeq (, @$(shell which $(CC)))
 $(warning "No $(CC) in path, consider installing $(CC)")
 CC := cc
endif
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

TEST_EXEC := galbt

$(TEST_EXEC): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)