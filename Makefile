CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

# Read version from VERSION file
VERSION := $(shell cat VERSION 2>/dev/null || echo "unknown")
CFLAGS += -DVERSION=\"$(VERSION)\"

# Platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    # macOS
    LDFLAGS += -framework CoreFoundation -framework CoreAudio -framework AudioToolbox
endif
ifeq ($(UNAME), Linux)
    # Linux
    LDFLAGS += -lpulse -lpthread
endif

SRC_DIR := src
BUILD_DIR := build
BIN := $(BUILD_DIR)/walcman

SOURCES := $(SRC_DIR)/main.c $(SRC_DIR)/player.c $(SRC_DIR)/input.c $(SRC_DIR)/util.c $(SRC_DIR)/error.c $(SRC_DIR)/terminal.c $(SRC_DIR)/ui_core.c $(SRC_DIR)/ui_format.c $(SRC_DIR)/ui_components.c $(SRC_DIR)/ui_screens.c $(SRC_DIR)/update.c
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(BUILD_DIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BIN) $(LDFLAGS)
	@echo "Build complete: $(BIN)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(BIN)
	./$(BIN)

install:
ifeq ($(UNAME), Darwin)
	@chmod +x installers/install-macos.sh
	@./installers/install-macos.sh
else ifeq ($(UNAME), Linux)
	@chmod +x installers/install-linux.sh
	@./installers/install-linux.sh
else
	@echo "Error: Installer not available for $(UNAME)"
	@echo "Supported platforms: macOS (Darwin), Linux"
	@exit 1
endif

uninstall:
ifeq ($(UNAME), Darwin)
	@chmod +x installers/uninstall-macos.sh
	@./installers/uninstall-macos.sh
else ifeq ($(UNAME), Linux)
	@chmod +x installers/uninstall-linux.sh
	@./installers/uninstall-linux.sh
else
	@echo "Error: Uninstaller not available for $(UNAME)"
	@echo "Supported platforms: macOS (Darwin), Linux"
	@exit 1
endif

.PHONY: all clean run install uninstall
