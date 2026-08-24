# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project

cmake_minimum_required(VERSION 3.15)
project(eFirmware
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "EoS device firmware toolkit — image container, integrity, packaging"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ====================================================================
# Options
# ====================================================================
# Tests default ON for host builds. A default of OFF is how a suite stops
# being run at all, so the only thing that disables them here is the absence
# of a machine to run them on.
if(CMAKE_CROSSCOMPILING)
    set(_efw_tests_default OFF)
else()
    set(_efw_tests_default ON)
endif()
option(EFW_BUILD_TESTS "Build and register unit tests" ${_efw_tests_default})
option(EFW_BUILD_TOOLS "Build the efwtool host utility"  ON)
option(EFW_HARDENING   "Enable compiler hardening flags" ON)
option(EFW_WERROR      "Treat warnings as errors"        ON)
option(EFW_SANITIZE    "Enable ASAN/UBSAN for host builds" OFF)

# ====================================================================
# Compiler flags
# ====================================================================
if(MSVC)
    add_compile_options(/W4)
    if(EFW_WERROR)
        add_compile_options(/WX)
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow
                        -Wconversion -Wsign-conversion
                        -Wstrict-prototypes -Wmissing-prototypes)
    if(EFW_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

if(EFW_HARDENING AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fstack-protector-strong)
    # _FORTIFY_SOURCE needs optimisation to do anything and warns without it.
    add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>)
endif()

if(EFW_SANITIZE AND NOT CMAKE_CROSSCOMPILING
   AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# ====================================================================
# Library
# ====================================================================
add_library(efw STATIC
    src/efw_sha256.c
    src/efw_crc32.c
    src/efw_image.c
)
target_include_directories(efw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(efw PROPERTIES
    VERSION   ${PROJECT_VERSION}
    C_VISIBILITY_PRESET hidden
)

# ====================================================================
# Tools and tests
# ====================================================================
if(EFW_BUILD_TOOLS)
    add_subdirectory(tools/efwtool)
endif()

if(EFW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ====================================================================
# Summary
# ====================================================================
message(STATUS "--------------------------------------------")
message(STATUS "eFirmware ${PROJECT_VERSION}")
message(STATUS "  Tests:      ${EFW_BUILD_TESTS}")
message(STATUS "  Tools:      ${EFW_BUILD_TOOLS}")
message(STATUS "  Hardening:  ${EFW_HARDENING}")
message(STATUS "  Warn=Error: ${EFW_WERROR}")
message(STATUS "  Sanitizers: ${EFW_SANITIZE}")
message(STATUS "--------------------------------------------")
