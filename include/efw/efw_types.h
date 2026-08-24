// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// ISO/IEC 25000 | ISO/IEC/IEEE 15288:2023

/**
 * @file efw_types.h
 * @brief Shared scalar types, status codes and version encoding for eFirmware.
 *
 * The values here deliberately mirror eBoot's include/eos_types.h so that an
 * image produced by this repository is byte-compatible with the bootloader
 * that consumes it. Changing a constant below is an on-the-wire format change
 * and must be matched in eBoot.
 */

#ifndef EFW_TYPES_H
#define EFW_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- Status codes ----------------
 * Identical numbering to eos_types.h: a status may cross the eFirmware/eBoot
 * boundary unchanged. */

#define EFW_OK                 0
#define EFW_ERR_GENERIC       -1
#define EFW_ERR_INVALID       -2
#define EFW_ERR_CRC           -3
#define EFW_ERR_SIGNATURE     -4
#define EFW_ERR_NO_IMAGE      -5
#define EFW_ERR_FLASH         -6
#define EFW_ERR_TIMEOUT       -7
#define EFW_ERR_BUSY          -8
#define EFW_ERR_AUTH          -9
#define EFW_ERR_VERSION      -10
#define EFW_ERR_FULL         -11
#define EFW_ERR_NOT_FOUND    -12
#define EFW_ERR_NOT_SUPPORTED -13
#define EFW_ERR_ANTI_ROLLBACK -14
#define EFW_ERR_REPLAY       -15
#define EFW_ERR_DECRYPT      -16
#define EFW_ERR_KEY          -17
#define EFW_ERR_DEBUG_LOCKED -18

/**
 * @brief Human-readable name for a status code.
 * @param status Any EFW_OK / EFW_ERR_* value.
 * @return A static, never-NULL string; "ERR_UNKNOWN" for unrecognised values.
 *
 * @code
 * printf("%s\n", efw_status_str(EFW_ERR_CRC));  // prints "ERR_CRC"
 * @endcode
 */
const char *efw_status_str(int status);

/* ---------------- Version encoding ----------------
 * Packed as 0xMMmmpppp: 8-bit major, 8-bit minor, 16-bit patch. */

#define EFW_VERSION_MAKE(major, minor, patch) \
    ((((uint32_t)(major) & 0xFFu) << 24) |    \
     (((uint32_t)(minor) & 0xFFu) << 16) |    \
     ((uint32_t)(patch) & 0xFFFFu))

#define EFW_VERSION_MAJOR(v) (((uint32_t)(v) >> 24) & 0xFFu)
#define EFW_VERSION_MINOR(v) (((uint32_t)(v) >> 16) & 0xFFu)
#define EFW_VERSION_PATCH(v) ((uint32_t)(v) & 0xFFFFu)

#ifdef __cplusplus
}
#endif

#endif /* EFW_TYPES_H */
