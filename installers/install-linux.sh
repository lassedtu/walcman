#!/bin/bash

# basically copied from install-macos.shell

# Configuration
INSTALL_DIR="$HOME/.local/bin/"
CONFIG_DIR="$HOME/.config/walcman"
CONFIG_FILE="$CONFIG_DIR/config"
VERSION_FILE="$CONFIG_DIR/VERSION"
ALIAS_NAME="walcman"
ZSHRC_FILE="$HOME/.zshrc"

# Get the directory where the install script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Get the project root directory (parent of installers/)
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

#INSTALL_DIR="$PROJECT_ROOT/test/"

echo Installing at $INSTALL_DIR

# Build the project
echo "Building walcman..."
cd "$PROJECT_ROOT"

# Check if VERSION file exists
if [ ! -f "$PROJECT_ROOT/VERSION" ]; then
    echo -e "Error: VERSION file not found"
    exit 1
fi

# Read current version
CURRENT_VERSION=$(cat "$PROJECT_ROOT/VERSION" | tr -d '\n')

echo "Installing version: $CURRENT_VERSION"

# Build the project (fallback to makefile with fixed flags)
echo "Building walcman..."
cd "$PROJECT_ROOT"

# not sure exactly why this would be called first but i will make it as close to the macos installation as possible
make clean > /dev/null 2>&1 || true

make

if [ ! -f "$PROJECT_ROOT/build/walcman" ]; then
    echo -e "${RED}Error: Build failed. walcman binary not found${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful${NC}" # nice ai checkmark symbol in the original lol
echo ""


# Create installation directory
echo "Installing to $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"
mkdir -p "$CONFIG_DIR"

# Copy files to install directory
cp "$PROJECT_ROOT/build/walcman" "$INSTALL_DIR/"
cp "$PROJECT_ROOT/VERSION" "$VERSION_FILE"

# Make binary executable
chmod +x "$INSTALL_DIR/walcman"

echo -e "${GREEN}Installation complete${NC}"
echo ""

# Create config file with defaults if it doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
    cat > "$CONFIG_FILE" << 'CONFIG_EOF'
# walcman configuration
update_check_enabled=1
check_interval_hours=24

# UI color (color name)
# Leave empty for default terminal color
# Options: red, green, yellow, blue, magenta, purple, pink, cyan, white, gray, orange
ui_color=
CONFIG_EOF
    echo -e "${GREEN}Config file created${NC}"
else
    echo -e "${GREEN}Config file exists${NC}"
fi

# Setup shell alias in ~/.zshrc
echo "Configuring shell alias in $ZSHRC_FILE..."
ALIAS_LINE="alias $ALIAS_NAME=\"$INSTALL_DIR/walcman\""

if [ ! -f "$ZSHRC_FILE" ]; then
    touch "$ZSHRC_FILE"
fi

if grep -q "alias $ALIAS_NAME=" "$ZSHRC_FILE"; then
    if grep -Fxq "$ALIAS_LINE" "$ZSHRC_FILE"; then
        echo "Alias already configured"
    else
        grep -v "alias $ALIAS_NAME=" "$ZSHRC_FILE" > "$ZSHRC_FILE.tmp"
        echo "$ALIAS_LINE" >> "$ZSHRC_FILE.tmp"
        mv "$ZSHRC_FILE.tmp" "$ZSHRC_FILE"
        echo "Alias updated"
    fi
else
    echo "$ALIAS_LINE" >> "$ZSHRC_FILE"
    echo "Alias added"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -"Installation complete."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "To use walcman on Linux:"
echo "  1. Build manually: make"
echo "  2. Run: ./build/walcman"
echo ""
echo "Then run: walcman <audio-file>"
echo ""
