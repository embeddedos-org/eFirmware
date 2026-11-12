# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project

# The harness lives in this directory, so every test needs it on the path.
foreach(_t sha256 crc32 image abi)
    add_executable(efw_test_${_t} test_${_t}.c)
    target_link_libraries(efw_test_${_t} PRIVATE efw)
    target_include_directories(efw_test_${_t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME efw_${_t} COMMAND efw_test_${_t})
endforeach()

# efwtool command-line checks. A malformed --load/--entry/--flags/--version
# must be refused with exit code 2 and no image written, not truncated or
# partly parsed into a wrong header field. efwtool_cli.cmake checks the exit
# code, the message and the output file for each case.
if(TARGET efwtool)
    set(_efw_cli ${CMAKE_CURRENT_SOURCE_DIR}/efwtool_cli.cmake)
    set(_efw_payload ${CMAKE_CURRENT_SOURCE_DIR}/efw_test.h)

    add_test(NAME efwtool_pack_accepts_valid_args
             COMMAND ${CMAKE_COMMAND}
                     -DEFWTOOL=$<TARGET_FILE:efwtool> -DPAYLOAD=${_efw_payload}
                     -DOUT=${CMAKE_CURRENT_BINARY_DIR}/efwtool_accepts_valid_args.img
                     -DEXPECT=accept -P ${_efw_cli})

    foreach(_case
            "load_trailing_text|--load|0x0801000G"
            "load_over_32_bits|--load|0x108010000"
            "load_negative|--load|-1"
            "load_bare_0x|--load|0x"
            "load_bad_octal|--load|08"
            "entry_trailing_text|--entry|8010100h"
            "entry_leading_plus|--entry|+5"
            "flags_trailing_text|--flags|0x4z"
            "version_trailing_text|--version|1.2.3x"
            "version_four_parts|--version|1.2.3.4")
        string(REPLACE "|" ";" _parts "${_case}")
        list(GET _parts 0 _name)
        list(GET _parts 1 _opt)
        list(GET _parts 2 _val)
        add_test(NAME efwtool_pack_rejects_${_name}
                 COMMAND ${CMAKE_COMMAND}
                         -DEFWTOOL=$<TARGET_FILE:efwtool> -DPAYLOAD=${_efw_payload}
                         -DOUT=${CMAKE_CURRENT_BINARY_DIR}/efwtool_rejects_${_name}.img
                         -DEXPECT=reject -DOPT=${_opt} "-DVAL=${_val}" -P ${_efw_cli})
    endforeach()

    # An image built from the largest payload pack accepts must also be
    # accepted by inspect and verify.
    add_test(NAME efwtool_max_size_payload_round_trip
             COMMAND ${CMAKE_COMMAND}
                     -DEFWTOOL=$<TARGET_FILE:efwtool>
                     -DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/efwtool_max_size.cmake)
endif()
