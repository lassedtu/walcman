# Installers

This directory contains platform-specific installation scripts for walcman.

## Available Installers

### macOS (Fully Implemented)
- **install-macos.sh** - Automated installer for macOS
  - Builds the project
  - Installs to `~/.config/walcman`
  - Adds shell alias to `~/.zshrc`
  - Version management with upgrade detection
- **uninstall-macos.sh** - Clean removal for macOS

### Linux (Placeholder - Coming Soon)
- **install-linux.sh** - Placeholder script
- **uninstall-linux.sh** - Placeholder script

Linux installers are planned for the future. Core playback functionality is experimental but should work with manual build.

### Windows (Placeholder - Coming Soon)
- **install-windows.bat** - Placeholder script
- **uninstall-windows.bat** - Placeholder script

Windows installers are planned for the future. Core playback functionality is experimental but should work with manual build.

## Usage

From the project root directory:

**macOS:**
```bash
make install    # Run the installer
make uninstall  # Remove walcman
```

**Linux/Windows:**
Currently use manual build:
```bash
make
./build/walcman  # Linux
build\walcman.exe # Windows
```

## Development

When implementing installers for other platforms:
- Follow the naming convention: `install-{platform}.sh` / `uninstall-{platform}.sh`
- Update the Makefile to detect and call the appropriate installer
- Test installation, upgrading, and uninstallation flows
- Document platform-specific requirements
