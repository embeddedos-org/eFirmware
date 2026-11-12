# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project
#
# The largest payload `efwtool pack` accepts (64 MiB) must give an image that
# `efwtool inspect` and `efwtool verify` accept too, although the image file is
# 156 header bytes bigger than the payload.
#   cmake -DEFWTOOL=<exe> -DWORKDIR=<dir> -P efwtool_max_size.cmake

set(_payload "${WORKDIR}/efwtool_max_size_payload.bin")
set(_image   "${WORKDIR}/efwtool_max_size.img")
file(REMOVE "${_payload}" "${_image}")

# 64 MiB of payload, written 1 MiB at a time.
string(REPEAT "e" 1024 _kib)
string(REPEAT "${_kib}" 1024 _mib)
file(WRITE "${_payload}" "")
foreach(_i RANGE 1 64)
    file(APPEND "${_payload}" "${_mib}")
endforeach()
file(SIZE "${_payload}" _size)
if(NOT _size EQUAL 67108864)
    message(FATAL_ERROR "test payload is ${_size} bytes, expected 67108864")
endif()

set(_failed "")
foreach(_step pack inspect verify)
    if(_step STREQUAL "pack")
        set(_args pack "${_payload}" "${_image}")
    else()
        set(_args ${_step} "${_image}")
    endif()
    execute_process(COMMAND "${EFWTOOL}" ${_args}
                    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    message(STATUS "efwtool ${_step}: exit ${rc} ${err}")
    if(NOT rc EQUAL 0)
        set(_failed "${_failed} ${_step}")
    endif()
    if(_step STREQUAL "pack" AND NOT rc EQUAL 0)
        break()
    endif()
endforeach()

file(REMOVE "${_payload}" "${_image}")
if(_failed)
    message(FATAL_ERROR "64 MiB payload: efwtool failed at:${_failed}")
endif()
