
SRC_DIR := src/
INCLUDES := -I$(SRC_DIR)
LIBS :=
TEST_FLAGS := -Wl,--defsym=main=__test_main -Wl,-wrap,abort -Wl,-wrap,assert
CFLAGS := -Wall -Wextra -Werror -std=gnu23 -g $(INCLUDES) $(LIBS) $(TEST_FLAGS)

INTERN_DEPS := std/

CC := clang
ifeq (, @$(shell which $(CC)))
 $(warning "No $(CC) in path, consider installing $(CC)")
 CC := cc
endif
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

TEST_EXEC := galbt

.PHONY: $(INTERN_DEPS)
$(INTERN_DEPS):
	@$(MAKE) -C $@ > /dev/null

$(TEST_EXEC): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

test: $(INTENR_DEPS) $(TEST_EXEC)
	./$(TEST_EXEC)
