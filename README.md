# walcman

A lightweight music and audio player that lives in your terminal. Built with C and miniaudio for minimal resource consumption and maximum performance.

## Features

### Current (v1.1.1)
- Play audio files (MP3, WAV, FLAC, M4A, OGG, AAC, WMA)
- Pause and resume playback
- Stop playback
- Single-key controls (no need to press Enter)
- Command-line file argument support
- Automatic detection when song ends
- Clean terminal UI with status display
- **Automated installer with version management (macOS)**

### Planned (v1.2.0+)
- Universal installer support (Linux, Windows)
- Drag-and-drop file support
- Progress bar with time display
- Shuffle playback
- EQ presets (load and create custom)
- Repeat modes
- Volume control
- Playlist management
- In-player file browser

## Why walcman?

walcman is built for users who want a music player that:
- Stays out of your way (runs in the terminal)
- Uses minimal system resources (powered by miniaudio and C)
- Gets the job done without bloat

## Platform Support

| Platform | Playback | Installer | Notes |
|----------|----------|-----------|-------|
| macOS    | Working | Available | Fully tested on macOS 10.13+ |
| Linux    | Experimental | Coming Soon | Core libraries compatible, needs testing |
| Windows  | Experimental | Coming Soon | Core libraries compatible, needs testing |

The Makefile includes platform detection and appropriate linker flags for all three platforms.

> **Note:** Universal installer support is planned for v1.2.0. Currently, Linux and Windows users should use manual build.

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

## Usage

### Keyboard Controls

walcman uses single-key input - just press the key, no need to hit Enter!

| Key | Action |
|-----|--------|
| **p** | Play a file (prompts for file path) |
| **Space** | Pause/Resume playback |
| **s** | Stop playback |
| **h** | Show help |
| **q** | Quit |

### Example Session

1. Start walcman:
```bash
./build/walcman
```

2. Press `p` to play a file, then type or paste the file path:
```
/Users/username/Music/song.mp3
```

3. Press `Space` to pause, `Space` again to resume

4. Press `s` to stop, or `q` to quit

### Playing from Command Line

You can also pass a file directly when starting:
```bash
./build/walcman ~/Music/favorite-song.mp3
```

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

### Current Development Focus

- **v1.2.0**: Drag-and-drop file support
- Progress bar with time tracking
- Platform testing (Linux and Windows)
- Enhanced UI features
- Playlist management system

## Project Status

- **Current Version**: v1.1.0
- **Status**: Active Development
- **Roadmap**: See [ROADMAP.md](ROADMAP.md) (coming soon)

## Acknowledgments

- [miniaudio](https://github.com/mackron/miniaudio) - Core audio library
- Inspired by lightweight terminal music players

