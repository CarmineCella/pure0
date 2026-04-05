cmake_minimum_required(VERSION 3.16)

project(pure0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Global compile definitions
# ------------------------------------------------------------------------------

add_compile_definitions(VERSION="0.1")
add_compile_definitions(COPYRIGHT="2026")

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(BUILD_PURE0_RTSOUND "Build pure0 realtime audio support" OFF)

# ------------------------------------------------------------------------------
# Realtime audio dependency (miniaudio)
# ------------------------------------------------------------------------------

if(BUILD_PURE0_RTSOUND)
    include(FetchContent)

    FetchContent_Declare(
        miniaudio
        URL https://github.com/mackron/miniaudio/archive/refs/heads/master.zip
    )

    FetchContent_GetProperties(miniaudio)
    if(NOT miniaudio_POPULATED)
        FetchContent_MakeAvailable(miniaudio)
    endif()

    add_compile_definitions(BUILD_PURE0_RTSOUND)
endif()

# ------------------------------------------------------------------------------
# Subprojects
# ------------------------------------------------------------------------------

add_subdirectory(src)

# ------------------------------------------------------------------------------
# pure0 uninstall target
# ------------------------------------------------------------------------------

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY
)

add_custom_target(pure0_uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    COMMENT "Uninstall pure0"
)