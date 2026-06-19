cmake_minimum_required(VERSION 3.31)

project(WXKB_SWITCH VERSION 1.0.3 LANGUAGES C)

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

# ── Installed file list (used by uninstall target) ───────────────────
# These paths are computed at configure time from CMAKE_INSTALL_PREFIX.
# The uninstall target works for the prefix used during configuration.
set(_WXKB_INSTALL_FILES
    "${CMAKE_INSTALL_FULL_BINDIR}/wxkb-switch"
    "${CMAKE_INSTALL_FULL_SYSCONFDIR}/bash_completion.d/wxkb-switch.sh"
    "${CMAKE_INSTALL_FULL_DATAROOTDIR}/zsh/vendor-completions/_wxkb-switch"
    "${CMAKE_INSTALL_FULL_DATAROOTDIR}/fish/vendor_completions.d/wxkb-switch.fish"
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
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d
)

install(FILES scripts/_wxkb-switch
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/zsh/vendor-completions
)

install(FILES scripts/wxkb-switch.fish
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/fish/vendor_completions.d
)

install(FILES man/wxkb-switch.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

# ── CPack (Debian package generation) ───────────────────────────────
set(CPACK_PACKAGE_NAME "wxkb-switch")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Frolov Ruslan Victorovich <gald.rus@yandex.ru>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "Utility for switching keyboard layouts under Wayland window system."
)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libc6, libx11-6, libxkbcommon-x11-0, libxkbfile1, libxcb1"
)
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
include(CPack)
