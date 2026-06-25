# ──────────────────────────────────────────────────────────────────────
# Dockerfile — wxkb-switch package builder
#
# Uses Debian trixie-slim as the base image for all architectures.
# Builds the binary and generates .deb / .rpm packages via CPack.
# Multi-arch support is provided by Docker Buildx + QEMU:
#   docker buildx build --platform linux/amd64,linux/arm64 ...
#
# ──────────────────────────────────────────────────────────────────────

FROM debian:trixie-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive

# ── Install build dependencies ────────────────────────────────────────
#
# build-essential — gcc, g++, make
# cmake            — build system (3.31+ from Debian trixie)
# pkg-config       — library discovery at configure time
# dpkg-dev         — utilities for .deb package generation
# rpm              — utility for .rpm package generation via CPack
#
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    pkg-config \
    dpkg-dev \
    rpm \
    && rm -rf /var/lib/apt/lists/*

# ── Install CMake and library dependencies ───────────────────────────
#
# libx11-dev           — X11 display connection (required)
# libxkbcommon-x11-dev — XKB keymap discovery via xkbcommon
# libxkbfile-dev       — XKB file format support
# libxcb1-dev          — XCB connection for keyboard device ID
#
RUN apt-get update && apt-get install -y --no-install-recommends \
    libx11-dev \
    libxkbcommon-x11-dev \
    libxkbfile-dev \
    libxcb1-dev \
    && rm -rf /var/lib/apt/lists/*

# ── Copy source and build ────────────────────────────────────────────
WORKDIR /src
COPY . .

# Configure CMake with /usr prefix (standard for packages)
RUN cmake --version && \
    cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr && \
    cmake --build build

# ── Generate packages (.deb and .rpm) ────────────────────────────────
#
# CPack is a built-in CMake module for generating installation packages.
# Supports DEB, RPM, TGZ, ZIP, and other formats.
RUN mkdir -p /out && \
    cpack --config build/CPackConfig.cmake -G DEB 2>&1 | tee /out/cpack-deb.log && \
    find /src -maxdepth 1 -name "wxkb-switch-*.deb" -exec mv {} /out/ \; && \
    cpack --config build/CPackConfig.cmake -G RPM 2>&1 | tee /out/cpack-rpm.log && \
    find /src -maxdepth 1 -name "wxkb-switch-*.rpm" -exec mv {} /out/ \;

# ── Verify build results ─────────────────────────────────────────────
RUN echo "=== Generated packages ===" && ls -lh /out/

# ── Final image: only packages (via --output type=local) ─────────────
FROM scratch AS output
COPY --from=builder /out /
