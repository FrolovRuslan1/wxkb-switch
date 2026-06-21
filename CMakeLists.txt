cmake_minimum_required(VERSION 3.31)

project(WXKB_SWITCH VERSION 1.0.4 LANGUAGES C)

# ── C23 standard with strict warnings ───────────────────────────────
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ── Detect package manager for install hints ────────────────────────
function(_detect_package_manager VAR)
    if(EXISTS "/usr/bin/apt")
        set(${VAR} "apt" PARENT_SCOPE)
    elseif(EXISTS "/usr/bin/dnf")
        set(${VAR} "dnf" PARENT_SCOPE)
    elseif(EXISTS "/usr/bin/pacman")
        set(${VAR} "pacman" PARENT_SCOPE)
    elseif(EXISTS "/usr/bin/zypper")
        set(${VAR} "zypper" PARENT_SCOPE)
    else()
        set(${VAR} "unknown" PARENT_SCOPE)
    endif()
endfunction()

_detect_package_manager(_PKG_MANAGER)

# ── pkg-config (required for dependency discovery) ──────────────────
find_package(PkgConfig REQUIRED)
if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "\n"
        "pkg-config not found. Install it first:\n"
        "  sudo apt install pkg-config"
    )
endif()

# ── Helper: pick correct package name for detected manager ───────────
function(_pick_pkg VAR apt_pkg dnf_pkg pacman_pkg zypper_pkg)
    if("${_PKG_MANAGER}" STREQUAL "apt")
        set(${VAR} "${apt_pkg}" PARENT_SCOPE)
    elseif("${_PKG_MANAGER}" STREQUAL "dnf")
        set(${VAR} "${dnf_pkg}" PARENT_SCOPE)
    elseif("${_PKG_MANAGER}" STREQUAL "pacman")
        set(${VAR} "${pacman_pkg}" PARENT_SCOPE)
    elseif("${_PKG_MANAGER}" STREQUAL "zypper")
        set(${VAR} "${zypper_pkg}" PARENT_SCOPE)
    else()
        set(${VAR} "${apt_pkg}" PARENT_SCOPE)
    endif()
endfunction()

# ── Helper: check one dependency, fail with install hint ─────────────
# pc_var  = CMake variable prefix (e.g. X11)
# pc_name = pkg-config module name  (e.g. x11)
function(_check_dep pc_var pc_name apt_pkg dnf_pkg pacman_pkg zypper_pkg)
    pkg_check_modules(${pc_var} IMPORTED_TARGET ${pc_name})
    if(NOT ${pc_var}_FOUND)
        _pick_pkg(pkg "${apt_pkg}" "${dnf_pkg}" "${pacman_pkg}" "${zypper_pkg}")

        # Build a one-line install command for all deps (fallback)
        if("${_PKG_MANAGER}" STREQUAL "apt")
            set(ALL_DEPS_CMD "sudo apt install libx11-dev libxkbfile-dev libxkbcommon-x11-dev libxcb1-dev")
        elseif("${_PKG_MANAGER}" STREQUAL "dnf")
            set(ALL_DEPS_CMD "sudo dnf install libX11-devel libxkbfile-devel xkbcommon-x11-devel libxcb-devel")
        elseif("${_PKG_MANAGER}" STREQUAL "pacman")
            set(ALL_DEPS_CMD "sudo pacman -S libx11 libxkbfile xkbcommon libxcb")
        else()
            set(ALL_DEPS_CMD "Install the development package for ${pc_name} manually.")
        endif()

        message(FATAL_ERROR
            "Package '${pc_name}' not found via pkg-config."
            "\n\n"
            "Install the missing dependency:"
            "\n"
            "  sudo ${_PKG_MANAGER} install ${pkg}"
            "\n\n"
            "Or install all dependencies at once:"
            "\n"
            "  ${ALL_DEPS_CMD}"
        )
    endif()
endfunction()

# Check each dependency individually with clear error messages
# _check_dep(<cmake_var> <pkg-config_name> <apt_pkg> <dnf_pkg> <pacman_pkg> <zypper_pkg>)
_check_dep(X11           x11            "libx11-dev"              "libX11-devel"               "libx11"            "libX11-devel")
_check_dep(XKBFILE       xkbfile        "libxkbfile-dev"          "libxkbfile-devel"           "libxkbfile"        "libxkbfile-devel")
_check_dep(XKBCOMMON_X11 xkbcommon-x11  "libxkbcommon-x11-dev"    "xkbcommon-x11-devel"        "xkbcommon"         "xkbcommon-x11-devel")
_check_dep(XCB           xcb            "libxcb1-dev"             "libxcb-devel"               "libxcb"            "libxcb-devel")

# ── Executable target (explicit sources, no GLOB) ───────────────────
add_executable(wxkb-switch
    src/main.c
)

# Embed version at compile time
target_compile_definitions(wxkb-switch PRIVATE
    WXKBSWITCH_VERSION="${PROJECT_VERSION}"
)

# Link dependencies (IMPORTED_TARGET provides include paths automatically)
target_link_libraries(wxkb-switch PRIVATE
    PkgConfig::X11
    PkgConfig::XKBFILE
    PkgConfig::XKBCOMMON_X11
    PkgConfig::XCB
)

# ── Install (GNUInstallDirs for portable paths) ─────────────────────
include(GNUInstallDirs)

# ── Configurable shell completion directories ────────────────────────
#
# These are CACHE variables — override with -D on the CMake command line.
#
# The defaults work out-of-the-box for standard prefixes (/usr, /usr/local).
# For non-standard prefixes or packaging scenarios, you may need to adjust
# them so that each shell can find the completion files in its expected
# search path (e.g. zsh $fpath, bash-completion load paths).
#
# Examples:
#
#   Standard /usr/local install (default):
#     cmake -S . -B build
#
#   System-wide /usr prefix with proper completion paths:
#     cmake -S . -B build \
#           -DCMAKE_INSTALL_PREFIX=/usr \
#           -DWXKB_ZSH_COMPLETION_DIR=/usr/share/zsh/site-functions
#
#   Non-standard prefix (e.g. NixOS, chroot):
#     cmake -S . -B build \
#           -DCMAKE_INSTALL_PREFIX=/opt/myapp \
#           -DWXKB_ZSH_COMPLETION_DIR=/usr/share/zsh/site-functions
#
set(WXKB_BASH_COMPLETION_DIR
    "${CMAKE_INSTALL_FULL_SYSCONFDIR}/bash_completion.d"
    CACHE PATH "Directory for bash completion scripts (default: ${CMAKE_INSTALL_FULL_SYSCONFDIR}/bash_completion.d)"
)
set(WXKB_ZSH_COMPLETION_DIR
    "${CMAKE_INSTALL_FULL_DATAROOTDIR}/zsh/site-functions"
    CACHE PATH "Directory for zsh completion functions (must be in $fpath; default: ${CMAKE_INSTALL_FULL_DATAROOTDIR}/zsh/site-functions)"
)
set(WXKB_FISH_COMPLETION_DIR
    "${CMAKE_INSTALL_FULL_DATAROOTDIR}/fish/vendor_completions.d"
    CACHE PATH "Directory for fish completion scripts (default: ${CMAKE_INSTALL_FULL_DATAROOTDIR}/fish/vendor_completions.d)"
)

# ── Print install summary ────────────────────────────────────────────
message(STATUS "Install prefix:        ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Binary:              ${CMAKE_INSTALL_FULL_BINDIR}/wxkb-switch")
message(STATUS "  Bash completion:     ${WXKB_BASH_COMPLETION_DIR}/wxkb-switch.sh")
message(STATUS "  Zsh completion:      ${WXKB_ZSH_COMPLETION_DIR}/_wxkb-switch")
message(STATUS "  Fish completion:     ${WXKB_FISH_COMPLETION_DIR}/wxkb-switch.fish")
message(STATUS "  Man page:            ${CMAKE_INSTALL_FULL_MANDIR}/man1/wxkb-switch.1")

# ── Installed file list (used by uninstall target) ───────────────────
# These paths are computed at configure time from CMAKE_INSTALL_PREFIX.
# The uninstall target works for the prefix used during configuration.
set(_WXKB_INSTALL_FILES
    "${CMAKE_INSTALL_FULL_BINDIR}/wxkb-switch"
    "${WXKB_BASH_COMPLETION_DIR}/wxkb-switch.sh"
    "${WXKB_ZSH_COMPLETION_DIR}/_wxkb-switch"
    "${WXKB_FISH_COMPLETION_DIR}/wxkb-switch.fish"
    "${CMAKE_INSTALL_FULL_MANDIR}/man1/wxkb-switch.1"
)

# ── Uninstall target (generates script at configure time) ────────────
file(WRITE "${CMAKE_BINARY_DIR}/uninstall-wxkb.cmake"
     "message(\"=== Uninstalling wxkb-switch ===\")\n"
)
foreach(_f IN LISTS _WXKB_INSTALL_FILES)
    file(APPEND "${CMAKE_BINARY_DIR}/uninstall-wxkb.cmake"
         "if(EXISTS \"${_f}\")\n"
         "  file(REMOVE \"${_f}\")\n"
         "  message(\"  removed: ${_f}\")\n"
         "else()\n"
         "  message(\"  not found (ok): ${_f}\")\n"
         "endif()\n"
    )
endforeach()

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/uninstall-wxkb.cmake"
)

# ── Actual install commands (relative destinations for DESTDIR/CPack) ─
install(TARGETS wxkb-switch
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS scripts/wxkb-switch.sh
        DESTINATION ${WXKB_BASH_COMPLETION_DIR}
)

install(FILES scripts/_wxkb-switch
        DESTINATION ${WXKB_ZSH_COMPLETION_DIR}
)

install(FILES scripts/wxkb-switch.fish
        DESTINATION ${WXKB_FISH_COMPLETION_DIR}
)

install(FILES man/wxkb-switch.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

# ── CPack (package generation: DEB, RPM) ────────────────────────────

# Common settings
set(CPACK_PACKAGE_NAME "wxkb-switch")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Utility for switching keyboard layouts under Wayland window system."
)
set(CPACK_PACKAGE_VENDOR "Frolov Ruslan Victorovich <gald.rus@yandex.ru>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "gald.rus@yandex.ru")

# Source tarball generators
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

# ── Debian (.deb) settings ───────────────────────────────────────────
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Frolov Ruslan Victorovich <gald.rus@yandex.ru>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "Utility for switching keyboard layouts under Wayland window system."
)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libc6, libx11-6, libxkbcommon-x11-0, libxkbfile1, libxcb1"
)

# ── RPM (.rpm) settings ──────────────────────────────────────────────
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "System Environment/Daemons")
set(CPACK_RPM_PACKAGE_REQUIRES
    "libX11, libxkbcommon-x11, libxkbfile, libxcb"
)
set(CPACK_RPM_AUTOREQPROVE ON)

include(CPack)
