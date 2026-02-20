#!/bin/bash

# walcman installer for macOS
# This script installs walcman to ~/.config/walcman and sets up a shell alias

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="$HOME/.config/walcman"
ZSHRC_FILE="$HOME/.zshrc"
ALIAS_NAME="walcman"

# Get the directory where the install script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  walcman installer"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check if VERSION file exists
if [ ! -f "$SCRIPT_DIR/VERSION" ]; then
    echo -e "${RED}Error: VERSION file not found${NC}"
    exit 1
fi

# Read current version
CURRENT_VERSION=$(cat "$SCRIPT_DIR/VERSION" | tr -d '\n')
echo "Installing version: $CURRENT_VERSION"
echo ""

# Check for existing installation
if [ -d "$INSTALL_DIR" ] && [ -f "$INSTALL_DIR/VERSION" ]; then
    INSTALLED_VERSION=$(cat "$INSTALL_DIR/VERSION" | tr -d '\n')
    echo "Found existing installation: v$INSTALLED_VERSION"
    
    if [ "$INSTALLED_VERSION" = "$CURRENT_VERSION" ]; then
        echo -e "${YELLOW}Same version already installed. Reinstalling...${NC}"
    else
        echo -e "${GREEN}Upgrading from v$INSTALLED_VERSION to v$CURRENT_VERSION${NC}"
    fi
    echo ""
fi

# Build the project
echo "Building walcman..."
cd "$SCRIPT_DIR"
make clean > /dev/null 2>&1 || true
make

if [ ! -f "$SCRIPT_DIR/build/walcman" ]; then
    echo -e "${RED}Error: Build failed. walcman binary not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"
echo ""

# Create installation directory
echo "Installing to $INSTALL_DIR..."
mkdir -p "$INSTALL_DIR"

# Copy files
cp "$SCRIPT_DIR/build/walcman" "$INSTALL_DIR/"
cp "$SCRIPT_DIR/VERSION" "$INSTALL_DIR/"

# Make binary executable
chmod +x "$INSTALL_DIR/walcman"

echo -e "${GREEN}✓ Files installed${NC}"
echo ""

# Setup shell alias
echo "Configuring shell alias..."

# The alias command to add
ALIAS_LINE="alias $ALIAS_NAME=\"$INSTALL_DIR/walcman\""

# Create .zshrc if it doesn't exist
if [ ! -f "$ZSHRC_FILE" ]; then
    echo "Creating $ZSHRC_FILE..."
    touch "$ZSHRC_FILE"
fi

# Check if alias already exists
if grep -q "alias $ALIAS_NAME=" "$ZSHRC_FILE"; then
    # Alias exists, check if it's the same
    if grep -Fxq "$ALIAS_LINE" "$ZSHRC_FILE"; then
        echo -e "${GREEN}✓ Alias already configured correctly${NC}"
    else
        # Update the alias by removing old line and adding new one
        # Use a temporary file to avoid issues with in-place editing
        grep -v "alias $ALIAS_NAME=" "$ZSHRC_FILE" > "$ZSHRC_FILE.tmp"
        echo "" >> "$ZSHRC_FILE.tmp"
        echo "# walcman alias" >> "$ZSHRC_FILE.tmp"
        echo "$ALIAS_LINE" >> "$ZSHRC_FILE.tmp"
        mv "$ZSHRC_FILE.tmp" "$ZSHRC_FILE"
        echo -e "${GREEN}✓ Alias updated${NC}"
    fi
else
    # Alias doesn't exist, add it
    echo "" >> "$ZSHRC_FILE"
    echo "# walcman alias" >> "$ZSHRC_FILE"
    echo "$ALIAS_LINE" >> "$ZSHRC_FILE"
    echo -e "${GREEN}✓ Alias added${NC}"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${GREEN}Installation complete!${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "To start using walcman, either:"
echo "  1. Restart your terminal"
echo "  2. Run: source ~/.zshrc"
echo ""
echo "Then run: walcman <audio-file>"
echo ""
