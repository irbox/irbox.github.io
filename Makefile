CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c99
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
