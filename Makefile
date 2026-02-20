CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

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

SOURCES := $(SRC_DIR)/main.c $(SRC_DIR)/player.c $(SRC_DIR)/input.c $(SRC_DIR)/ui.c $(SRC_DIR)/util.c $(SRC_DIR)/error.c $(SRC_DIR)/terminal.c
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

.PHONY: all clean run
