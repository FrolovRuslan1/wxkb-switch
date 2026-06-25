#!/usr/bin/env bash
#
# build-all-packages.sh — Build .deb and .rpm packages for all supported architectures.
#
# Uses Docker Buildx with QEMU emulation to build native packages per architecture.
# No cross-compilers required on the host system.
#
# Usage:
#   ./scripts/build-all-packages.sh [OPTIONS]
#
# Options:
#   --arch ARCH        Build only the specified architecture (comma-separated)
#   --output DIR       Output directory for packages (default: ./packages)
#   --jobs N           Number of parallel builds (default: $(nproc))
#   --no-cache         Do not use Docker build cache
#   --help             Show this help message
#
# Example:
#   ./scripts/build-all-packages.sh --arch x86_64,aarch64
#
# Build all architectures in parallel (default):
#   ./scripts/build-all-packages.sh
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# ── Defaults ─────────────────────────────────────────────────────────
OUTPUT_DIR="$PROJECT_ROOT/packages"
FILTER_ARCH=""
NO_CACHE=false
JOBS=$(nproc 2>/dev/null || echo 4)
BUILDX_BUILDER="wxkb-builder"
LOG_DIR=""

# ── Architecture definitions ─────────────────────────────────────────
# Format: NAME|DOCKER_PLATFORM
#
# Derived from: docker buildx imagetools inspect debian:trixie-slim
# CPack auto-detects target architecture inside the container,
# so separate dpkg/rpm columns are unnecessary.
ARCHITECTURES=(
	"x86_64|linux/amd64"
	"i386|linux/386"
	"aarch64|linux/arm64"
	"armv5l|linux/arm/v5"
	"armv7l|linux/arm/v7"
	"riscv64|linux/riscv64"
	"ppc64le|linux/ppc64le"
	"s390x|linux/s390x"
)

# ── Functions ────────────────────────────────────────────────────────

usage() {
	head -25 "$0" | grep '^#' | sed 's/^# \?//'
	exit 0
}

log_info() { echo "[INFO]  $*"; }
log_ok() { echo "[OK]    $*"; }
log_warn() { echo "[WARN]  $*" >&2; }
log_error() { echo "[ERROR] $*" >&2; }

# ── Dependency checks ────────────────────────────────────────────────

check_dependencies() {
	local missing=0

	for cmd in docker; do
		if ! command -v "$cmd" &>/dev/null; then
			log_error "Required command not found: $cmd"
			missing=1
		fi
	done

	if ! docker buildx version >/dev/null 2>&1; then
		log_error "Docker Buildx not available. Install it with:"
		echo "  sudo apt install docker-buildx-plugin"
		missing=1
	fi

	if ! command -v qemu-user-static &>/dev/null &&
		! dpkg -l qemu-user-static 2>/dev/null | grep -q '^ii'; then
		log_error "QEMU user static not found. Required for non-native architectures."
		echo "Install with: sudo apt install qemu-user-static binfmt-support"
		missing=1
	fi

	if ! docker info >/dev/null 2>&1; then
		log_error "Docker daemon is not running or current user lacks permissions."
		echo "Start Docker and ensure your user is in the 'docker' group:"
		echo "  sudo systemctl start docker"
		echo "  sudo usermod -aG docker \$USER"
		missing=1
	fi

	if [ "$missing" -eq 1 ]; then
		exit 1
	fi

	log_ok "All dependencies satisfied"
}

# Ensure QEMU binfmt is registered inside Docker for all architectures
setup_qemu() {
	log_info "Registering QEMU binfmt handlers..."
	if docker run --rm --privileged multiarch/qemu-user-static --reset -p yes >/dev/null 2>&1; then
		log_ok "QEMU binfmt ready"
	else
		log_warn "QEMU setup failed — non-native architectures may not work"
	fi
}

# Create a buildx builder with multi-platform support
setup_buildx() {
	if ! docker buildx inspect "$BUILDX_BUILDER" --bootstrap >/dev/null 2>&1; then
		log_info "Creating buildx builder: $BUILDX_BUILDER"
		docker buildx create --use --name "$BUILDX_BUILDER" --driver docker-container \
			--driver-opt network=host 2>/dev/null || true
	fi

	docker buildx use "$BUILDX_BUILDER" 2>/dev/null || true
	docker buildx inspect --bootstrap >/dev/null 2>&1 || true
	log_ok "Buildx builder ready"
}

# Build packages for a single architecture using Docker
build_arch() {
	local arch_name="$1"
	local platform="$2"
	local log_file="$LOG_DIR/${arch_name}.log"
	# Use per-architecture temp directory to avoid race conditions on shared OUTPUT_DIR
	local build_output="$BUILD_TMP/${arch_name}"
	mkdir -p "$build_output"

	log_info "Building ${arch_name} (${platform})..."

	# Build arguments
	local build_args=(--platform "$platform" -f "$PROJECT_ROOT/Dockerfile")
	if $NO_CACHE; then
		build_args+=(--no-cache)
	fi

	# Run the build — output goes to isolated per-arch directory
	if docker buildx build "${build_args[@]}" \
		--output "type=local,dest=$build_output" \
		"$PROJECT_ROOT" 2>&1 | tee "$log_file"; then
		# Rename packages with architecture suffix to avoid collisions
		local found=false
		for pkg in "$build_output"/wxkb-switch-*.deb "$build_output"/wxkb-switch-*.rpm; do
			[ -e "$pkg" ] || continue
			local base="${pkg%.*}"
			local ext="${pkg##*.}"
			# Rename: wxkb-switch-X.Y.Z-Linux.deb → wxkb-switch-X.Y.Z-${arch_name}.deb
			mv "$pkg" "${base}-${arch_name}.${ext}"
			log_ok "  $(basename "${base}-${arch_name}.${ext}")"
			found=true
		done
		$found || log_warn "  No new packages found for ${arch_name}"
		return 0
	else
		log_error "Build failed for ${arch_name} (log: $log_file)"
		return 1
	fi
}

# ── Argument parsing ────────────────────────────────────────────────

while [[ $# -gt 0 ]]; do
	case "$1" in
	--arch)
		[ $# -ge 2 ] || {
			log_error "--arch requires a value"
			exit 1
		}
		FILTER_ARCH="$2"
		shift 2
		;;
	--output)
		[ $# -ge 2 ] || {
			log_error "--output requires a value"
			exit 1
		}
		OUTPUT_DIR="$2"
		shift 2
		;;
	--no-cache)
		NO_CACHE=true
		shift
		;;
	--jobs)
		[ $# -ge 2 ] || {
			log_error "--jobs requires a value"
			exit 1
		}
		if ! [[ "$2" =~ ^[0-9]+$ ]] || [ "$2" -le 0 ]; then
			log_error "--jobs must be a positive integer, got: $2"
			exit 1
		fi
		JOBS="$2"
		shift 2
		;;
	--help | -h)
		usage
		;;
	*)
		log_error "Unknown option: $1"
		usage
		;;
	esac
done

# ── Validation ───────────────────────────────────────────────────────

# ── Check all dependencies before starting builds ────────────────────
check_dependencies

# ── Main ─────────────────────────────────────────────────────────────

mkdir -p "$OUTPUT_DIR"
LOG_DIR=$(mktemp -d "${TMPDIR:-/tmp}/wxkb-logs-XXXXXX")
BUILD_TMP=$(mktemp -d "${TMPDIR:-/tmp}/wxkb-build-XXXXXX")
trap 'rm -rf "$LOG_DIR" "$BUILD_TMP"' EXIT

log_info "Output directory: $OUTPUT_DIR"
log_info "Host architecture: $(uname -m)"
log_info "Build logs:       $LOG_DIR"
echo

# Setup Docker environment for multi-arch builds
setup_qemu
setup_buildx
echo

# Build an associative array of valid architectures for O(1) lookup
declare -A VALID_ARCHS
for entry in "${ARCHITECTURES[@]}"; do
	name="${entry%%|*}"
	VALID_ARCHS["$name"]=1
done

# Parse filter — validate each requested architecture against known list
if [ -n "$FILTER_ARCH" ]; then
	IFS=',' read -ra FILTER_LIST <<<"$FILTER_ARCH"
	for f in "${FILTER_LIST[@]}"; do
		if [ -z "${VALID_ARCHS[$f]+x}" ]; then
			log_error "Unknown architecture: $f (valid: ${!VALID_ARCHS[*]})"
			exit 1
		fi
	done
else
	# No filter — build everything
	FILTER_LIST=("${!VALID_ARCHS[@]}")
fi

# Collect architectures to build (preserve declaration order)
BUILD_LIST=()
for entry in "${ARCHITECTURES[@]}"; do
	name="${entry%%|*}"
	for f in "${FILTER_LIST[@]}"; do
		if [ "$name" = "$f" ]; then
			BUILD_LIST+=("$entry")
			break
		fi
	done
done

if [ ${#BUILD_LIST[@]} -eq 0 ]; then
	log_error "No architectures to build"
	exit 1
fi

SKIPPED_COUNT=$((${#ARCHITECTURES[@]} - ${#BUILD_LIST[@]}))

log_info "Building ${#BUILD_LIST[@]} architecture(s) with $JOBS parallel job(s)..."
echo

# ── Parallel build ───────────────────────────────────────────────────
#
# Launch background jobs respecting --jobs limit.
# Track exit codes via per-arch status files for reliable counting.

PIDS=()

for entry in "${BUILD_LIST[@]}"; do
	IFS='|' read -r name platform <<<"$entry"

	# Throttle: wait until we have a free slot
	while [ $(jobs -rp | wc -l) -ge "$JOBS" ]; do
		sleep 0.5
	done

	(
		if build_arch "$name" "$platform"; then
			echo "0" >"$LOG_DIR/${name}.status"
		else
			echo "1" >"$LOG_DIR/${name}.status"
		fi
	) &
	PIDS+=("$!")
done

# Wait for all jobs; count failures from status files
FAIL_COUNT=0
for pid in "${PIDS[@]}"; do
	wait "$pid" 2>/dev/null || true
done

for entry in "${BUILD_LIST[@]}"; do
	name="${entry%%|*}"
	if [ -f "$LOG_DIR/${name}.status" ]; then
		status=$(cat "$LOG_DIR/${name}.status")
		if [ "$status" != "0" ]; then
			log_error "Failed: $name"
			((FAIL_COUNT++)) || true
		fi
	else
		log_error "No status for: $name (likely crashed)"
		((FAIL_COUNT++)) || true
	fi
done

SUCCESS_COUNT=$((${#BUILD_LIST[@]} - FAIL_COUNT))

# ── Summary ──────────────────────────────────────────────────────────

echo
log_info "═══════════════════════════════════════════"
log_info "Build complete!"
log_info "  Successful: $SUCCESS_COUNT"
log_info "  Failed:     $FAIL_COUNT"
log_info "  Skipped:    $SKIPPED_COUNT"
log_info "  Output dir: $OUTPUT_DIR"
log_info "═══════════════════════════════════════════"
echo

# Copy all renamed packages from per-arch directories to OUTPUT_DIR
for entry in "${BUILD_LIST[@]}"; do
	name="${entry%%|*}"
	local_arch_dir="$BUILD_TMP/${name}"
	if [ -d "$local_arch_dir" ]; then
		mv "$local_arch_dir"/*.deb "$local_arch_dir"/*.rpm "$OUTPUT_DIR/" 2>/dev/null || true
	fi
done

if [ -d "$OUTPUT_DIR" ]; then
	log_info "Generated packages:"
	ls -lh "$OUTPUT_DIR/" | grep -E '\.(deb|rpm)$' || echo "  (no packages generated)"
fi

exit $FAIL_COUNT
