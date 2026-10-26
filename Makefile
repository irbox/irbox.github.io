CC ?= cc
C_STD ?= $(shell $(CC) -std=c2y -E -x c /dev/null >/dev/null 2>&1 && echo -std=c2y || ($(CC) -std=c23 -E -x c /dev/null >/dev/null 2>&1 && echo -std=c23 || echo -std=c2x))
LTO ?= $(shell $(CC) -flto -E -x c /dev/null >/dev/null 2>&1 && echo -flto || echo "")
HARDENING ?= $(shell $(CC) -fstack-protector-strong -E -x c /dev/null >/dev/null 2>&1 && echo -fstack-protector-strong || echo "")
CFLAGS ?= -O3 -Wall -Wextra -Wpedantic $(C_STD) $(LTO) $(HARDENING)
SRC = tools/ssg.c
BIN ?= /tmp/irbox-ssg
OUT ?= _site

.PHONY: all build clean check

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

build: $(BIN)
	@$(BIN) $(OUT)
	@echo "Build complete in $(OUT)/"

check:
	$(CC) -std=c23 -Wall -Wextra -Wpedantic -fsyntax-only $(SRC)
	@echo "C23 syntax check passed with 0 warnings."

clean:
	rm -rf $(BIN) $(OUT)

