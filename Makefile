# SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
# SPDX-License-Identifier: MIT

CC       := clang
CLIENT	 := sdl3
PREFIX   := /usr/local
TARGET   := cgbl

CFLAGS   := -march=native -std=c23 -Wall -Werror -Wextra -Wno-unused-parameter -MMD -MP -flto=auto -fpie -O3 -DNDEBUG \
-DCLIENT_$(CLIENT) -DPATCH=0x$(shell git rev-parse --short HEAD)
LDFLAGS  := $(shell pkg-config readline $(CLIENT) --cflags)
LDLIBS   := $(shell pkg-config readline $(CLIENT) --libs)

INCLUDES := $(shell find src -type d | sed "s/^/-I/")
HEADERS  := $(shell find src -name "*.h")
SOURCES  := $(shell find src -name "*.c")

DEPS     := $(SOURCES:.c=.d)
OBJECTS  := $(SOURCES:.c=.o)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(DEPS) $(OBJECTS) $(TARGET)

.PHONY: format
format:
	clang-format -i $(SOURCES) $(HEADERS)

.PHONY: install
install: $(TARGET)
	install -D -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	strip --strip-all -R .note -R .comment $(PREFIX)/bin/$(TARGET)
	install -D -m 644 docs/$(TARGET).1 $(PREFIX)/share/man/man1/$(TARGET).1
	gzip -f $(PREFIX)/share/man/man1/$(TARGET).1

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	rm -f $(PREFIX)/share/man/man1/$(TARGET).1.gz

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -c $< -o $@

-include $(DEPS)
