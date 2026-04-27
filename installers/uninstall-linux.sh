#!/bin/bash

set -e

INSTALL_DIR="$HOME/.local/bin/"
ALIAS_NAME="walcman"
TARGET_BIN="$INSTALL_DIR/$ALIAS_NAME"

ZSHRC_FILE="$HOME/.zshrc"
BASHRC_FILE="$HOME/.bashrc"

remove_alias_from_file() {
	local rc_file="$1"

	if [ ! -f "$rc_file" ]; then
		return
	fi

	if grep -q "alias $ALIAS_NAME=" "$rc_file"; then
		grep -v "alias $ALIAS_NAME=" "$rc_file" > "$rc_file.tmp"
		mv "$rc_file.tmp" "$rc_file"
		echo "Removed alias from $rc_file"
	fi
}

echo "walcman Linux uninstaller"
echo "Binary scheduled for removal: $TARGET_BIN"
echo ""

if [ ! -f "$TARGET_BIN" ]; then
	echo "No walcman binary found at $TARGET_BIN"
else
	read -r -p "Continue? (y/N) " REPLY
	if [[ ! "$REPLY" =~ ^[Yy]$ ]]; then
		echo "Uninstall cancelled"
		exit 0
	fi

	rm -f "$TARGET_BIN"
	echo "Removed: $TARGET_BIN"
fi

# Remove legacy config file location created by the current Linux installer.
if [ -f "$INSTALL_DIR/config" ]; then
	rm -f "$INSTALL_DIR/config"
	echo "Removed: $INSTALL_DIR/config"
fi

remove_alias_from_file "$ZSHRC_FILE"
remove_alias_from_file "$BASHRC_FILE"

echo ""
echo "Uninstall complete"
echo "If you use zsh or bash, reload your shell config to apply alias changes."
