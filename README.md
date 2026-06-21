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
wget https://github.com/FrolovRuslan1/wxkb-switch/releases/download/v1.0.4/wxkb-switch-1.0.4-Linux.deb
sudo apt install ./wxkb-switch-1.0.4-Linux.deb
```

### From .rpm package (Fedora/RHEL/openSUSE)

Download the latest release from [GitHub Releases](https://github.com/FrolovRuslan1/wxkb-switch/releases):
```bash
wget https://github.com/FrolovRuslan1/wxkb-switch/releases/download/v1.0.4/wxkb-switch-1.0.4-Linux.x86_64.rpm
sudo dnf install ./wxkb-switch-1.0.4-Linux.x86_64.rpm
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

**Customize installation paths (optional):**

Shell completion scripts can be installed to custom directories via CMake variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `WXKB_BASH_COMPLETION_DIR` | `${PREFIX}/etc/bash_completion.d` | Bash completion script location |
| `WXKB_ZSH_COMPLETION_DIR` | `${PREFIX}/share/zsh/site-functions` | Zsh completion function location |
| `WXKB_FISH_COMPLETION_DIR` | `${PREFIX}/share/fish/vendor_completions.d` | Fish completion script location |

Example — install binary to `/opt/myapp` but completions to system directories:
```bash
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=/opt/myapp \
    -DWXKB_BASH_COMPLETION_DIR=/etc/bash_completion.d \
    -DWXKB_ZSH_COMPLETION_DIR=/usr/share/zsh/site-functions
```

## Building Packages

### .deb Package (Debian/Ubuntu)

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

### .rpm Package (Fedora/RHEL/openSUSE)

To generate your own `.rpm` package from source:

```bash
# Configure with /usr prefix (standard for packages)
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr

# Build
cmake --build build

# Generate the .rpm package
cpack --config build/CPackConfig.cmake -G RPM
```

The resulting `.rpm` file will be placed in the project root directory.
You can install it with:
```bash
# Fedora/RHEL
sudo dnf install ./wxkb-switch-*.rpm

# openSUSE
sudo zypper install ./wxkb-switch-*.rpm
```

> **Note:** Building RPM packages requires `rpmbuild` to be installed.
> On Fedora: `sudo dnf install rpm-build`. On RHEL: `sudo yum install rpm-build`.

## Usage

```bash
wxkb-switch [options]
```

| Command | Options | Description |
|---------|---------|-------------|
| Next layout | `wxkb-switch` / `-n` / `--next` | Switch to the next keyboard layout |
| Previous layout | `-p` / `--prev` | Switch to the previous keyboard layout |
| List layouts | `-l` / `--list` | Show all available layouts (current marked with `<---`) |
| Version | `-v` / `--version` | Print program version |
| Debug mode | `-d` / `--debug` | Enable verbose output (combine with other options) |
| GNOME override | `-g` / `--gnome` | Force execution under GNOME |
| Help | `-h` / `--help` | Display help message |

### Examples

```bash
# Switch to next layout (default action)
wxkb-switch

# Switch to previous layout
wxkb-switch --prev

# List all layouts (current marked with <---)
wxkb-switch --list
```

Output example:
```
0: English (US) <--- current layout
1: Russian
```

```bash
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

### From .rpm package
```bash
sudo dnf remove wxkb-switch
# or on older systems:
sudo rpm -e wxkb-switch
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
| GNOME (X11/Wayland) | Not supported — GNOME's settings-daemon manages layouts independently and may override external changes |

## License

See [LICENSE](LICENSE) for details.
