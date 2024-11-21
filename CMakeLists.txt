cmake_minimum_required(VERSION 3.10)

project(WXKB_SWITCH C CXX)
set(MAJOR_VERSION 0)
SET(MINOR_VERSION 0)
SET(PATCH_VERSION 1)
SET(WXKBSWITCH_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${RELEASE_VERSION})

# Check pkg-config 
find_package(PkgConfig)
if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found, please install 'pkg-config' or check it existing.")
endif()

# Check presence of development libraries required for build
# Check libxkbcommon library
pkg_check_modules(X11           REQUIRED  IMPORTED_TARGET x11)
pkg_check_modules(XKBFILE       REQUIRED  IMPORTED_TARGET xkbfile)
pkg_check_modules(XKBCOMMON     REQUIRED  IMPORTED_TARGET xkbcommon)
pkg_check_modules(XKBCOMMON_X11 REQUIRED  IMPORTED_TARGET xkbcommon-x11)
pkg_check_modules(XCB           REQUIRED  IMPORTED_TARGET xcb)
pkg_check_modules(XKBREGISTRY   REQUIRED  IMPORTED_TARGET xkbregistry)

# Handle error: library not found
if(NOT X11_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libx11' required for build. (Install libx11-dev or libx11-devel package.) CMake will exit.")
endif()
if(NOT XKBFILE_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libxkbfile' required for build. (Install libxkbfile-dev or libxkbfile-devel package.) CMake will exit.")
endif()
if(NOT XKBCOMMON_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libxkbcommon' required for build. CMake will exit.")
endif()
if(NOT XKBCOMMON_X11_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libxkbcommon-x11' required for build. CMake will exit.")
endif()
if(NOT XCB_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libxcb' required for build. CMake will exit.")
endif()
if(NOT XKBREGISTRY_FOUND)
    MESSAGE(FATAL_ERROR "Not found development files of 'libxkbregistry' required for build. CMake will exit.")
endif()

# Compile and link program
include_directories(${X11_INCLUDE_DIR})
include_directories(${XKBFILE_INCLUDE_DIR})
include_directories(${XKBCOMMON_INCLUDE_DIRS})
include_directories(${XKBCOMMON_X11_INCLUDE_DIRS})
include_directories(${XCB_INCLUDE_DIRS})
include_directories(${XKBREGISTRY_INCLUDE_DIRS})

link_directories(${X11_LIBRARY_DIR})
link_directories(${XKBFILE_LIBRARY_DIR})
link_directories(${XKBCOMMON_LIBRARY_DIRS})
link_directories(${XKBCOMMON_X11_LIBRARY_DIRS})
link_directories(${XCB_LIBRARY_DIRS})
link_directories(${XKBREGISTRY_LIBRARY_DIRS})

file(GLOB ALL_SRCS
          "${PROJECT_SOURCE_DIR}/include/*.h"
          "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(wxkb_switch ${ALL_SRCS})
# set_target_properties(wxkb_switch PROPERTIES 
#                                   VERSION WXKBSWITCH_VERSION)
set(external_libs PkgConfig::X11 
                  PkgConfig::XKBFILE 
                  PkgConfig::XKBCOMMON 
                  PkgConfig::XKBCOMMON_X11 
                  PkgConfig::XCB 
                  PkgConfig::XKBREGISTRY)
target_link_libraries(wxkb_switch ${external_libs})

# Install program
INSTALL(TARGETS wxkb_switch
        RUNTIME DESTINATION bin)

INSTALL(PROGRAMS scripts/wxkb_switch.sh
        DESTINATION /etc/bash_completion.d)

INSTALL(FILES scripts/_wxkb_switch
        DESTINATION /usr/share/zsh/vendor-completions)

INSTALL(FILES scripts/wxkb_switch.fish
        DESTINATION /usr/share/fish/vendor_completions.d)

install(FILES man/wxkb_switch.1
        DESTINATION share/man/man1)

# make uninstall
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with install_manifest.txt"
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)

# cpack deb package
set(CPACK_PACKAGE_NAME "wxkb_switcher")
set(CPACK_PACKAGE_VERSION ${WXKBSWITCH_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Frolov Ruslan Victorovich")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Utility for switching keyboard layouts under Wayland window system.")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libx11-6, libxkbcommon0, libxkbcommon-x11-0, libxcb1, libxkbregistry0, libxkbfile1")

include(CPack)