# wxkb-switch

Utility for switching keyboard layouts under Wayland and X11 window systems.
Reads the current XKB configuration directly — no setup required.

## Features

- Switch to next or previous keyboard layout
- List all available layouts with current one marked
- Works on Wayland (via XWayland/XKB) and most X11 desktop environments
- Shell completion for bash, zsh, and fish
- Man page included
- Debug mode for troubleshooting

## Requirements

| Library | Purpose |
|---------|--------|
| libX11 | X11 display connection |
| libxkbcommon-x11 | XKB keymap discovery via xkbcommon |
| libxkbfile | XKB file format support |
| libxcb | XCB connection for keyboard device ID |

## Installation

### From .deb package (Debian/Ubuntu)

Download the latest release from [GitHub Releases](https://github.com/FrolovRuslan1/wxkb-switch/releases):
```bash
wget https://github.com/FrolovRuslan1/wxkb-switch/releases/download/v1.0.3/wxkb-switch-1.0.3-Linux.deb
sudo apt install ./wxkb-switch-1.0.3-Linux.deb
```

### From source (recommended)

**Install build dependencies:**
```bash
# Debian/Ubuntu
sudo apt install build-essential cmake pkg-config \
    libx11-dev libxkbcommon-x11-dev libxkbfile-dev libxcb1-dev

# Fedora
sudo dnf install gcc make cmake pkgconf-pkg-config \
    libX11-devel xkbcommon-x11-devel libxkbfile-devel libxcb-devel
```

**Build and install:**
```bash
git clone https://github.com/FrolovRuslan1/wxkb-switch.git
cd wxkb-switch
mkdir build && cd build
cmake ..
make
sudo make install
```

## Building a .deb Package

To generate your own `.deb` package from source:

```bash
# Configure with /usr prefix (standard for packages)
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr

# Build
cmake --build build

# Generate the .deb package
cpack --config build/CPackConfig.cmake -G DEB
```

The resulting `.deb` file will be placed in the project root directory.
You can install it with:
```bash
sudo apt install ./wxkb-switch-*.deb
```

## Usage

```bash
wxkb-switch [options]
```

| Command | Options | Description |
|---------|---------|-------------|
| Next layout | `wxkb-switch` / `-n` / `--next` | Switch to the next keyboard layout |
| Previous layout | `-p` / `--prev` | Switch to the previous keyboard layout |
| List layouts | `-l` / `--list` | Show all available layouts and current one |
| Version | `-v` / `--version` | Print program version |
| Debug mode | `-d` / `--debug` | Enable verbose output (combine with other options) |
| Help | `-h` / `--help` | Display help message |

### Examples

```bash
# Switch to next layout (default action)
wxkb-switch

# Switch to previous layout
wxkb-switch --prev

# List all layouts (current marked with arrow)
wxkb-switch --list

# Debug output when switching
wxkb-switch -d --next
```

### Shell Integration

The package installs completion scripts for bash, zsh, and fish automatically.
Reload your shell or source the completion file to start using tab completion:
```bash
# Bash — usually loaded automatically via /etc/bash_completion.d/
source /etc/bash_completion.d/wxkb-switch.sh
```

## Uninstalling

### From .deb package
```bash
sudo apt remove wxkb-switch
```

### From source build
```bash
cd wxkb-switch/build
sudo make uninstall
```

## Compatibility

| Environment | Status |
|-------------|--------|
| Wayland (all) | Supported |
| X11 (most DEs) | Supported |
| GNOME on X11 | Not supported |

## License

See [LICENSE](LICENSE) for details.
