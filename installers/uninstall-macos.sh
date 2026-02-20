#!/bin/bash

# walcman uninstaller for macOS

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

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  walcman uninstaller"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check if installed
if [ ! -d "$INSTALL_DIR" ]; then
    echo -e "${YELLOW}walcman is not installed at $INSTALL_DIR${NC}"
    exit 0
fi

# Get installed version if available
if [ -f "$INSTALL_DIR/VERSION" ]; then
    INSTALLED_VERSION=$(cat "$INSTALL_DIR/VERSION" | tr -d '\n')
    echo "Found installation: v$INSTALLED_VERSION"
    echo ""
fi

# Confirm uninstall
read -p "Are you sure you want to uninstall walcman? (y/N) " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstall cancelled"
    exit 0
fi

# Remove installation directory
echo "Removing $INSTALL_DIR..."
rm -rf "$INSTALL_DIR"
echo -e "${GREEN}✓ Installation directory removed${NC}"

# Remove alias from .zshrc
if [ -f "$ZSHRC_FILE" ]; then
    echo "Removing alias from $ZSHRC_FILE..."
    
    if grep -q "alias $ALIAS_NAME=" "$ZSHRC_FILE"; then
        # Remove the alias line and the comment line before it if it exists
        grep -v "alias $ALIAS_NAME=" "$ZSHRC_FILE" | \
        grep -v "^# walcman alias$" > "$ZSHRC_FILE.tmp"
        mv "$ZSHRC_FILE.tmp" "$ZSHRC_FILE"
        echo -e "${GREEN}✓ Alias removed${NC}"
    else
        echo "No alias found in $ZSHRC_FILE"
    fi
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${GREEN}Uninstall complete!${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "To apply changes, run: source ~/.zshrc"
echo ""
