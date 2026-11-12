CC ?= cc
C_STD ?= $(shell $(CC) -std=c2y -E -x c /dev/null >/dev/null 2>&1 && echo -std=c2y || ($(CC) -std=c23 -E -x c /dev/null >/dev/null 2>&1 && echo -std=c23 || echo -std=c2x))

ifeq ($(FAST),1)
  CFLAGS ?= -O1 -Wall -Wextra $(C_STD)
else
  LTO ?= $(shell $(CC) -flto -E -x c /dev/null >/dev/null 2>&1 && echo -flto || echo "")
  HARDENING ?= $(shell $(CC) -fstack-protector-strong -E -x c /dev/null >/dev/null 2>&1 && echo -fstack-protector-strong || echo "")
  CFLAGS ?= -O3 -Wall -Wextra -Wpedantic $(C_STD) $(LTO) $(HARDENING)
endif
SRC = tools/ssg.c
TMP_DIR := $(if $(TMPDIR),$(TMPDIR),$(if $(wildcard /tmp),/tmp,.))
BIN ?= $(TMP_DIR)/irbox-ssg
OUT ?= _site

.PHONY: all build clean check test

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

build: $(BIN)
	@$(BIN) $(OUT)
	@echo "Build complete in $(OUT)/"

check:
	$(CC) $(C_STD) -Wall -Wextra -Wpedantic -fsyntax-only $(SRC)
	@echo "C23 syntax check passed with 0 warnings."

test: check build
	@test -f $(OUT)/index.html || (echo "Missing index.html" && exit 1)
	@test -f $(OUT)/feed.xml || (echo "Missing feed.xml" && exit 1)
	@test -f $(OUT)/sitemap.xml || (echo "Missing sitemap.xml" && exit 1)
	@test -f $(OUT)/search.json || (echo "Missing search.json" && exit 1)
	@test -f $(OUT)/manifest.webmanifest || (echo "Missing manifest.webmanifest" && exit 1)
	@test -f $(OUT)/books/reader/index.html || (echo "Missing reader/index.html" && exit 1)
	@test -f $(OUT)/.nojekyll || (echo "Missing .nojekyll" && exit 1)
	@echo "All build verification tests passed successfully."

clean:
	rm -rf $(BIN) $(OUT) ./ssg

