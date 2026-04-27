#!/bin/bash

# basically copied from install-macos.shell

# Configuration
INSTALL_DIR="$HOME/.local/bin/"
ALIAS_NAME="walcman"

# Get the directory where the install script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Get the project root directory (parent of installers/)
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

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

# Check for existing installation (test this after installing)
if [ -d "$INSTALL_DIR" ] && [ -f "$INSTALL_DIR/VERSION" ]; then
    INSTALLED_VERSION=$(cat "$INSTALL_DIR/VERSION" | tr -d '\n')
    echo "Found existing installation: v$INSTALLED_VERSION"
    
    if [ "$INSTALLED_VERSION" = "$CURRENT_VERSION" ]; then
        echo -e "${YELLOW}Same version already installed. Reinstalling...${NC}"
    else
        echo -e "${GREEN}Upgrading from v$INSTALLED_VERSION to v$CURRENT_VERSION${NC}"
    fi
    echo ""
else
    echo "Installing version: $CURRENT_VERSION"
fi

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

