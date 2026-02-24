# walcman

A lightweight music and audio player that lives in your terminal. Built with C and miniaudio for minimal resource consumption and maximum performance.

## Features

- Play MP3, WAV, FLAC, M4A, OGG, AAC, WMA
- Single-key controls (no Enter required)
- File argument support
- Auto-detect end of playback
- macOS installer with version management
- Optional non-blocking auto-update check

## Platform Support

| Platform | Status       | Installer |
| -------- | ------------ | --------- |
| macOS    | Stable       | Yes       |
| Linux    | Experimental | No        |
| Windows  | Experimental | No        |

Linux and Windows require a manual build.

---

## Installation

### macOS

```bash
git clone https://github.com/lassedtu/walcman.git
cd walcman
make install
```

Restart your terminal or source your shell config to update `PATH`:

```bash
source ~/.zshrc
```

To uninstall:

```bash
make uninstall
```

### Linux / Windows (Manual Build)

**Requirements:** C compiler (clang or gcc), `make`, POSIX environment

```bash
git clone https://github.com/lassedtu/walcman.git
cd walcman
make
./build/walcman
```

---

## Usage

> **Note:** If installed via the macOS installer, run `walcman` from any directory. If built manually, use `./build/walcman`.

```bash
walcman
```

To play a file directly:

```bash
walcman /path/to/song.mp3
```

### Controls

| Key     | Action         |
| ------- | -------------- |
| `p`     | Play file      |
| `Space` | Pause / Resume |
| `s`     | Stop           |
| `r`     | Toggle repeat  |
| `h`     | Help           |
| `c`     | Toggle controls|
| `o`     | Open settings  |
| `q`     | Quit           |

---

## Configuration

> Only available when installed via the macOS installer.

Config file: `~/.config/walcman/config`

| Option                 | Values      | Description                            |
| ---------------------- | ----------- | -------------------------------------- |
| `update_check_enabled` | `1` / `0`   | Enable or disable update checks        |
| `check_interval_hours` | Integer     | How often to check for updates (hours) |
| `ui_color`             | Color name  | Color for entire UI text (optional)    |

Example:

```
update_check_enabled=1
check_interval_hours=24
ui_color=cyan
```

Available colors:
- `red`, `green`, `yellow`, `blue`  
- `pink`, `magenta`, `purple` (magenta and purple are aliases for pink)
- `cyan`, `white`, `gray`, `orange`
- Leave empty for default terminal color

---

## Contributing

Contributions are welcome! Fork the repo, make your changes on a new branch, and open a Pull Request.

## Acknowledgments

- [miniaudio](https://github.com/mackron/miniaudio): core audio library
- **Ana**: for the original concept and for helping refine the direction of the project from its earliest stages.