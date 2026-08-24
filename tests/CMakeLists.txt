# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project

# The harness lives in this directory, so every test needs it on the path.
foreach(_t sha256 crc32 image)
    add_executable(efw_test_${_t} test_${_t}.c)
    target_link_libraries(efw_test_${_t} PRIVATE efw)
    target_include_directories(efw_test_${_t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME efw_${_t} COMMAND efw_test_${_t})
endforeach()
