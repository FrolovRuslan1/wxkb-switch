# wxkb-switch

Utility for switching keyboard layouts under Wayland and X11 window systems.
Reads the current XKB configuration directly — no setup required.

## Features

- Switch to next or previous keyboard layout
- List all available layouts with current one marked
- Works on Wayland (limited — uses XWayland for XKB access; may not work with all compositors) and most X11 desktop environments
- Shell completion for bash, zsh, and fish
- Man page included
- Debug mode for troubleshooting

## Quick Start

```bash
# Install (Debian/Ubuntu)
sudo apt install ./wxkb-switch-*.deb

# Switch to next layout
wxkb-switch

# List available layouts
wxkb-switch --list
```

## Requirements

| Library | Purpose |
|---------|--------|
| libX11 | X11 display connection |
| libxkbcommon-x11 | XKB keymap discovery via xkbcommon |
| libxkbfile | XKB file format support |
| libxcb | XCB connection for keyboard device ID |

> **Note:** Minimum library versions are determined by your distribution's package manager.
> The build system will report specific version requirements if dependencies are too old.

## Installation

The recommended way to install wxkb-switch is via a pre-built package for your distribution.
Building from source is available as an alternative below.

### From .deb package (Debian/Ubuntu) — Recommended

Download the latest release from [GitHub Releases](https://github.com/FrolovRuslan1/wxkb-switch/releases):

```bash
wget https://github.com/FrolovRuslan1/wxkb-switch/releases/download/1.0.4/wxkb-switch-1.0.4-x86_64.deb
sudo apt install ./wxkb-switch-1.0.4-x86_64.deb
```

### From .rpm package (Fedora/RHEL/openSUSE) — Recommended

Download the latest release from [GitHub Releases](https://github.com/FrolovRuslan1/wxkb-switch/releases):

```bash
wget https://github.com/FrolovRuslan1/wxkb-switch/releases/download/1.0.4/wxkb-switch-1.0.4-x86_64.rpm
sudo dnf install ./wxkb-switch-1.0.4-x86_64.rpm
```

### From source (alternative)

Use this method if you need the latest development version or want to customize the build.

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

#### Customize installation paths (optional)

Shell completion scripts can be installed to custom directories via CMake variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `WXKB_BASH_COMPLETION_DIR` | `/etc/bash_completion.d` | Bash completion script location |
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

## Building Packages for All Architectures

wxkb-switch uses Docker Buildx with QEMU emulation to build native packages for multiple architectures. No cross-compilers are required on the host system.

### Prerequisites

Install Docker and required dependencies:

```bash
# Debian/Ubuntu
sudo apt install -y docker.io docker-buildx-plugin qemu-user-static binfmt-support

# Fedora
sudo dnf install -y docker buildah qemu-user-static binfmt-support
```

Ensure your user can run Docker without `sudo`:

```bash
sudo usermod -aG docker $USER
newgrp docker  # or log out and back in
```

### Quick Start

Build `.deb` and `.rpm` packages for all supported architectures:

```bash
./scripts/build-all-packages.sh
```

Packages are placed in `./packages/` by default. Builds run in parallel using all available CPU cores.

### Options

| Option | Description |
|--------|-------------|
| `--arch ARCH` | Build only specified architectures (comma-separated) |
| `--output DIR` | Output directory for packages (default: `./packages`) |
| `--jobs N` | Number of parallel builds (default: all CPU cores) |
| `--no-cache` | Do not use Docker build cache |

**Examples:**

```bash
# Build for aarch64 and riscv64 only
./scripts/build-all-packages.sh --arch aarch64,riscv64

# Build with 2 parallel jobs to reduce resource usage
./scripts/build-all-packages.sh --jobs 2

# Custom output directory
./scripts/build-all-packages.sh --output ./release
```

See `./scripts/build-all-packages.sh --help` for all options.

### How It Works

1. The script uses Docker Buildx to build inside Debian trixie containers
2. QEMU user-mode emulation handles non-native architectures
3. Each architecture builds independently in parallel
4. Generated packages are collected into the output directory

The `Dockerfile` in the project root contains the full build environment setup.

## Supported Architectures

| Architecture | Processor | Debian Package Arch | RPM Package Arch |
|-------------|-----------|---------------------|------------------|
| x86_64 (AMD64) | `x86_64` | `x86_64` | `x86_64` |
| ARM64 (AArch64) | `aarch64` | `arm64` | `aarch64` |
| ARM32 Hard Float | `armv7l` | `armhf` | `armv7hl` |
| RISC-V 64 | `riscv64` | `riscv64` | `riscv64` |
| PowerPC 64 LE | `ppc64le` | `ppc64el` | `ppc64le` |
| MIPS64 LE | `mips64el` | `mips64el` | `mips64` |
| s390x (IBM Z) | `s390x` | `s390x` | `s390x` |

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

```text
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

# Zsh — usually loaded automatically from /usr/share/zsh/site-functions/
source /usr/share/zsh/site-functions/_wxkb-switch

# Fish — usually loaded automatically from /usr/share/fish/vendor_completions.d/
source /usr/share/fish/vendor_completions.d/wxkb-switch.fish
```

## Uninstalling

### From .deb package

```bash
sudo apt remove wxkb-switch
```

### From .rpm package (Fedora/RHEL/openSUSE)

```bash
# Fedora/RHEL
sudo dnf remove wxkb-switch

# openSUSE
sudo zypper remove wxkb-switch

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
| GNOME (X11/Wayland) | Partially supported — GNOME's settings-daemon manages layouts independently; use `--gnome` flag to force execution, but changes may be overridden by GNOME settings daemon

## Troubleshooting

### Common Issues

**wxkb-switch cannot detect layouts:**
Run with debug mode to diagnose:

```bash
wxkb-switch -d --list
```

**Layout switch doesn't persist:**
Some desktop environments may override changes. Try the `--gnome` flag if using GNOME.

**Permission denied errors:**
Ensure your user has access to the X11 display. Set DISPLAY environment variable if needed:

```bash
export DISPLAY=:0
```

## Contributing

Bug reports and feature requests are welcome! Please open an issue at:
<https://github.com/FrolovRuslan1/wxkb-switch/issues>

See [Dockerfile](Dockerfile) for the full build environment setup.

## License

See [LICENSE](LICENSE) for details.
