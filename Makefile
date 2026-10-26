CC ?= cc
C_STD ?= $(shell $(CC) -std=c2y -E -x c /dev/null >/dev/null 2>&1 && echo -std=c2y || ($(CC) -std=c23 -E -x c /dev/null >/dev/null 2>&1 && echo -std=c23 || echo -std=c2x))
CFLAGS ?= -O3 -Wall -Wextra $(C_STD)
SRC = tools/ssg.c
BIN ?= /tmp/irbox-ssg
OUT ?= _site

.PHONY: all build clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

build: $(BIN)
	@$(BIN) $(OUT)
	@echo "Build complete in $(OUT)/"

clean:
	rm -rf $(BIN) $(OUT)

