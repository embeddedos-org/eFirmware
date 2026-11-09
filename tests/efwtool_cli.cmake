# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project
#
# Runs one `efwtool pack` case and checks exit code, message and output file.
#   cmake -DEFWTOOL=<exe> -DPAYLOAD=<file> -DOUT=<image> -DEXPECT=accept|reject
#         [-DOPT=--load -DVAL=<value>] -P efwtool_cli.cmake
# accept: valid arguments; must exit 0, print "packed" and write the image.
# reject: OPT VAL must exit 2, print "bad OPT" on stderr and write no image.

file(REMOVE "${OUT}")

if(EXPECT STREQUAL "accept")
    execute_process(
        COMMAND "${EFWTOOL}" pack "${PAYLOAD}" "${OUT}"
                --load 0x08010000 --entry 134283520 --flags 0 --version 1.2.3
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "valid arguments: exit ${rc}, expected 0\n${err}")
    endif()
    if(NOT out MATCHES "^packed ")
        message(FATAL_ERROR "valid arguments: no 'packed' line\n${out}")
    endif()
    if(NOT EXISTS "${OUT}")
        message(FATAL_ERROR "valid arguments: no image written")
    endif()
elseif(EXPECT STREQUAL "reject")
    execute_process(
        COMMAND "${EFWTOOL}" pack "${PAYLOAD}" "${OUT}" "${OPT}" "${VAL}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 2)
        message(FATAL_ERROR "${OPT} '${VAL}': exit ${rc}, expected 2\n${out}${err}")
    endif()
    if(NOT err MATCHES "bad ${OPT} ")
        message(FATAL_ERROR "${OPT} '${VAL}': no 'bad ${OPT}' message\n${err}")
    endif()
    if(EXISTS "${OUT}")
        message(FATAL_ERROR "${OPT} '${VAL}': an image was written")
    endif()
else()
    message(FATAL_ERROR "EXPECT must be accept or reject")
endif()
