# SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
# SPDX-License-Identifier: MIT

CC       := gcc
CLIENT	 := sdl2
CFLAGS   := -march=native -std=c99 -Wall -Werror -Wextra -Wno-unused-parameter -flto=auto -fpie -O3 -DNDEBUG -DCLIENT_$(CLIENT)
PREFIX   := /usr/local
TARGET   := cgbl

INCLUDES := $(subst src,-Isrc,$(shell find src -type d))
OBJECTS  := $(patsubst %.c,%.o,$(shell find src -name "*.c"))
PATCH    := $(shell git rev-parse --short HEAD)

ifeq ($(CLIENT),sdl2)
LDFLAGS  := $(shell pkg-config $(CLIENT) --cflags)
LDLIBS   := $(shell pkg-config $(CLIENT) --libs)
else ifeq ($(CLIENT),sdl3)
LDFLAGS  := $(shell pkg-config $(CLIENT) --cflags)
LDLIBS   := $(shell pkg-config $(CLIENT) --libs)
else
$(error Unsupported client -- $(CLIENT))
endif

.PHONY: all
all: patch $(TARGET)

.PHONY: clean
clean:
	@rm -f $(OBJECTS) $(TARGET)

.PHONY: install
install: install-bin install-docs

.PHONY: install-bin
install-bin:
	@mkdir -p $(PREFIX)/bin
	@cp -f $(TARGET) $(PREFIX)/bin
	@strip --strip-all -R .note -R .comment $(PREFIX)/bin/$(TARGET)

.PHONY: install-docs
install-docs:
	@mkdir -p $(PREFIX)/share/man/man1
	@gzip < docs/$(TARGET).1 > $(PREFIX)/share/man/man1/$(TARGET).1.gz

.PHONY: patch
patch:
	@sed -i "s/PATCH .*/PATCH 0x$(PATCH)/g" src/common.h

.PHONY: uninstall
uninstall: uninstall-bin uninstall-docs

.PHONY: uninstall-bin
uninstall-bin:
	@rm -f $(PREFIX)/bin/$(TARGET)

.PHONY: uninstall-docs
uninstall-docs:
	@rm -f $(PREFIX)/share/man/man1/$(TARGET).1.gz

$(TARGET): $(OBJECTS)
	@$(CC) $(CFLAGS) $(INCLUDES) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

%.o: %.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $< -o $@
