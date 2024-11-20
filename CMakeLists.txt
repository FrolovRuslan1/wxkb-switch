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
# set_target_properties(wxkb_switch PROPERTIES LINKER_LANGUAGE C)
set(external_libs PkgConfig::X11 
                  PkgConfig::XKBFILE 
                  PkgConfig::XKBCOMMON 
                  PkgConfig::XKBCOMMON_X11 
                  PkgConfig::XCB 
                  PkgConfig::XKBREGISTRY)
target_link_libraries(wxkb_switch ${external_libs})

# Install program
# INSTALL(TARGETS wxkb_switch
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION lib OPTIONAL
# )

# SET(MAN_COMPRESSION "gzip" CACHE STRING "Manpages compression tool")
# SET(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE STRING "Manpages installation path")

# Function to compress and install man page
# Gets file name and type number
# function(install_man man_filename man_type)
#     # check what compression tool is available
#     FIND_PROGRAM(COMPRESS_EXECUTABLE NAMES ${MAN_COMPRESSION})
#     if(NOT COMPRESS_EXECUTABLE)
#       SET(MAN_COMPRESSION NO)
#     endif()
#     # set input an output file names
#     SET(raw_man man/${man_filename}.${man_type})
#     # compress if there is the compression tool
#     if(MAN_COMPRESSION)
#       if(COMPRESS_EXECUTABLE MATCHES "gzip")
#           SET(installed_man ${CMAKE_BINARY_DIR}/${man_filename}.${man_type}.gz)
#         else()
#           SET(installed_man ${CMAKE_BINARY_DIR}/${man_filename}.${man_type}.${MAN_COMPRESSION})
#         endif()
#         ADD_CUSTOM_COMMAND(OUTPUT ${installed_man}
#             COMMAND cat ${raw_man} | ${COMPRESS_EXECUTABLE} > ${installed_man}
#             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#             DEPENDS ${raw_man}
#             COMMENT "Compressing man file ${raw_man} to ${installed_man}"
#         )
#     # elsewise just copy
#     else()
#         SET(installed_man ${CMAKE_BINARY_DIR}/${man_filename}.${man_type})
#         MESSAGE(WARNING "There is no compression tool for man pages. Not compressed copy of man file will be used.")
#         ADD_CUSTOM_COMMAND(OUTPUT ${installed_man}
#             COMMAND cp ${raw_man} ${CMAKE_BINARY_DIR}
#             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#             DEPENDS ${raw_man}
#             COMMENT "Copying man file from ${raw_man}."
#         )
#     endif()
#     # add actions
#     ADD_CUSTOM_TARGET(man_${man_filename}_${man_type} ALL DEPENDS ${installed_man})
#     INSTALL(FILES ${installed_man}
#         DESTINATION ${MANDIR}/man${man_type}
#     )
# endfunction()

# # Compress and install man page
# install_man(wxkb_switch 1)
