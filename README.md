# walcman

A lightweight music and audio player that lives in your terminal. Built with C and miniaudio for minimal resource consumption and maximum performance.

## Features

### Current (v1.2.0)
- Play audio files (MP3, WAV, FLAC, M4A, OGG, AAC, WMA)
- Single-key controls (no need to press Enter)
- Command-line file argument support
- Automatic detection when song ends
- Automated installer with version management (macOS)
- **Auto-update check on startup (non-blocking, silent)**

### Planned
- Progress bar with time display
- Volume control
- Shuffle playback
- Playlist management
- In-player file/directory browser
- EQ presets (load and create custom)
- Universal installer support (Linux, Windows)

## Why walcman?

walcman is built for users who want a music player that:
- Stays out of your way (runs in the terminal)
- Uses minimal system resources (powered by miniaudio and C)
- Just works, without bloat

## Platform Support

| Platform | Playback | Installer | Notes |
|----------|----------|-----------|-------|
| macOS    | Working | Available | Fully tested on macOS 10.13+ |
| Linux    | Experimental | Coming Soon | Core libraries compatible, needs testing |
| Windows  | Experimental | Coming Soon | Core libraries compatible, needs testing |

The Makefile includes platform detection and appropriate linker flags for all three platforms.

> **Note:** Universal installer support is planned in the future. Currently, Linux and Windows users should use manual build.

## Installation

### Prerequisites
- C compiler (clang on macOS, gcc on Linux)
- make

All dependencies (miniaudio) are bundled with the source.

### Automated Install (macOS Only)

> **macOS users:** Automated installer with version management is available!

1. Clone the repository:
```bash
git clone https://github.com/lassedtu/walcman.git
cd walcman
```

2. Run the installer:
```bash
make install
```

The installer will:
- Build the project
- Install walcman to `~/.config/walcman`
- Add a shell alias to `~/.zshrc`
- Check for existing installations and handle upgrades

3. Restart your terminal or run:
```bash
source ~/.zshrc
```

4. You can now use walcman from anywhere:
```bash
walcman /path/to/song.mp3
```

#### Uninstall (macOS)

To remove walcman:
```bash
make uninstall
```

#### Upgrading (macOS)

To upgrade to a new version:
1. Pull the latest changes: `git pull`
2. Run the installer again: `make install`

The installer automatically detects existing installations and handles upgrades.

### Manual Build (All Platforms)

> **Linux & Windows users:** Use manual build until automated installers are released.

1. Clone the repository:
```bash
git clone https://github.com/lassedtu/walcman.git
cd walcman
```

2. Build the project:
```bash
make
```

3. Run the player:
```bash
./build/walcman
```

### Starting the Player

**With automated installer (macOS):**
```bash
walcman                           # Interactive mode
walcman /path/to/song.mp3         # Direct playback
```

**With manual build (All platforms):**
```bash
./build/walcman                   # Interactive mode
./build/walcman /path/to/song.mp3 # Direct playback
```

**Interactive mode:**
Press `p` to select a file to play.

## Configuration

### Config File Location

On macOS (with automated installer), the config file is located at:
```
~/.config/walcman/config
```

The config file is created automatically during installation with default settings.

### Available Options

```bash
# Enable/disable automatic update checks on startup
update_check_enabled=1

# How often to check for updates (in hours)
check_interval_hours=24
```

### Modifying Configuration

Edit the config file directly with your preferred text editor:

```bash
vim ~/.config/walcman/config
```

Then modify the values:

- **`update_check_enabled`**: Set to `1` to enable auto-update checks, `0` to disable
- **`check_interval_hours`**: Number of hours between update checks

For example, to check for updates every 12 hours:
```bash
check_interval_hours=12
```

### Auto-Update Behavior

When `update_check_enabled=1`:
- walcman checks for new versions on startup (non-blocking, runs in background)
- If an update is available, it downloads and installs automatically
- No interruption to startup - the check happens in parallel
- If the check fails (no internet, GitHub down, etc.), walcman starts normally without errors
- Updates are atomic with rollback capability

## Usage

### Keyboard Controls

walcman uses single-key input - just press the key, no need to hit Enter.

| Key | Action |
|-----|--------|
| **p** | Play a file (prompts for file path) |
| **Space** | Pause/Resume playback |
| **s** | Stop playback |
| **h** | Show help |
| **r** | Toggle repeat |
| **c** | Toggle controls |
| **q** | Quit |

The player will start immediately and display playback controls.

## Technical Details

### Technology Stack
- **Language**: C (C99 standard)
- **Audio Backend**: miniaudio (single-header library)
- **Terminal Control**: POSIX termios for raw input mode
- **Build System**: Make with platform detection

### Architecture

walcman uses miniaudio as its core audio engine, providing:
- Cross-platform audio device abstraction
- Support for multiple audio formats (MP3, WAV, FLAC, OGG, etc.)
- Minimal dependencies and overhead
- Hardware-accelerated audio decoding

The project is modular with separate components:
- **player.c** - Audio playback engine
- **ui.c** - Terminal user interface
- **input.c** - Keyboard input handling
- **terminal.c** - Raw terminal mode control
- **error.c** - Error handling system
- **util.c** - String utilities

### Supported Audio Formats
- MP3
- WAV
- FLAC
- M4A
- OGG
- AAC
- WMA

## Requirements

### Build Requirements
- C compiler (clang/gcc)
- make
- Standard POSIX environment

### Runtime Requirements
- macOS 10.13+ (tested and working)
- Linux with PulseAudio (untested but should work)
- Windows 7+ (untested but should work)

## Development

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Commit your changes (`git commit -m 'Add some amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

## Acknowledgments

- [miniaudio](https://github.com/mackron/miniaudio) - Core audio library
- Inspired by lightweight terminal music players

