# walcman

A lightweight music and audio player that lives in your terminal. Built with C and miniaudio for minimal resource consumption and maximum performance.

> **Note:** v1.1.0 will include an installer that sets up `walcman` as a terminal command. For now, you'll need to run the binary directly from the build directory.

## Features

### Current (v1.0.0)
- Play audio files (MP3, WAV, FLAC, M4A, OGG, AAC, WMA)
- Pause and resume playback
- Stop playback
- Single-key controls (no need to press Enter)
- Command-line file argument support
- Automatic detection when song ends
- Clean terminal UI with status display

### Planned (v1.1.0+)
- Installer with terminal alias setup
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

| Platform | Status | Notes |
|----------|--------|-------|
| macOS    | Working | v1.0.0 fully tested on macOS 10.13+ |
| Linux    | Untested | Core libraries compatible, needs testing |
| Windows  | Untested | Core libraries compatible, needs testing |

The Makefile includes platform detection and appropriate linker flags for all three platforms.

## Installation

### Prerequisites
- C compiler (clang on macOS, gcc on Linux)
- make

All dependencies (miniaudio) are bundled with the source.

### Build from Source

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

Or build and run in one command:
```bash
make run
```

### Starting the Player

**Interactive mode:**
```bash
./build/walcman
```
Then press `p` to select a file to play.

**Direct playback:**
```bash
./build/walcman /path/to/your/song.mp3
```

### Future Versions

**v1.1.0** will include an installer script that:
- Installs the binary to your system
- Creates a terminal alias (`walcman`)
- Allows you to run the player from anywhere with just `walcman`

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

- **v1.1.0**: Installer script with terminal alias setup
- Drag-and-drop file support
- Progress bar with time tracking
- Platform testing (Linux and Windows)
- Enhanced UI features
- Playlist management system

## Project Status

- **Current Version**: v1.0.0
- **Status**: Active Development
- **Roadmap**: See [ROADMAP.md](ROADMAP.md) (coming soon)

## Acknowledgments

- [miniaudio](https://github.com/mackron/miniaudio) - Core audio library
- Inspired by lightweight terminal music players

