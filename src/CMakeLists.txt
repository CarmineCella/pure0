# src/CMakeLists.txt
#
# Command-line pure0 interpreter.

set(PURE0_SOURCES
    pure0.cpp
    ../src/core.h
    ../src/dsp.h
    ../src/music.h  
)

add_executable(pure0
    ${PURE0_SOURCES}
)

# Include the core headers
target_include_directories(pure0
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

if(BUILD_PURE0_RTSOUND)
    target_include_directories(pure0
        PRIVATE
            ${miniaudio_SOURCE_DIR}
    )
endif()

if(BUILD_PURE0_RTSOUND)
    target_compile_definitions(pure0
        PRIVATE
            BUILD_PURE0_RTSOUND
    )
endif()

# C++ standard and warnings/optimizations (per-target, not global)
target_compile_features(pure0 PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(pure0 PRIVATE /W4)
else()
    target_compile_options(pure0 PRIVATE
        -Wall
        -g
        -O2
        -Wunused-function
        -Wno-return-type-c-linkage
    )
endif()

if(APPLE AND BUILD_PURE0_RTSOUND)
    target_link_libraries(pure0 PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreFoundation"
    )
endif()

#
# Installation
#
# 1) Install the executable under "bin" in the install prefix.
# 2) Install .mu files into $HOME/.pure0
#    and, on Unix, chown them back to the invoking user if run via sudo.
#

# Install binary under ${CMAKE_INSTALL_PREFIX}/bin
install(TARGETS pure0
    RUNTIME DESTINATION bin
)

# Install .mu files into ~/.pure0
install(FILES
    ${CMAKE_SOURCE_DIR}/src/stdlib.scm
    DESTINATION $ENV{HOME}/.pure0
)

# On Unix: if install is run with sudo, chown the .mu files to $SUDO_USER
# so the normal user can overwrite them later from the IDE.
install(CODE "
    if(NOT WIN32)
      set(PURE0_HOME \"$ENV{HOME}\")

      if(DEFINED ENV{SUDO_USER} AND NOT \"\$ENV{SUDO_USER}\" STREQUAL \"\")
        set(PURE0_OWNER \"\$ENV{SUDO_USER}\")
      else()
        execute_process(
          COMMAND whoami
          OUTPUT_VARIABLE PURE0_OWNER
          OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      endif()

      file(GLOB mu_files \"\${PURE0_HOME}/.pure0/*.scm\")
      foreach(f \${mu_files})
        execute_process(
          COMMAND chown \${PURE0_OWNER} \"\${f}\"
        )
      endforeach()
    endif()
")