// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_crc32.c
 * @brief Table-free CRC-32/ISO-HDLC.
 *
 * The bitwise form is used deliberately: a 1 KiB lookup table is a poor trade
 * in a bootloader-adjacent image where flash is the scarce resource, and image
 * verification is not on any hot path.
 */

#include "efw/efw_crc32.h"

uint32_t efw_crc32_update(uint32_t crc, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t i;
    unsigned b;

    if (len == 0 || !p) return crc;

    crc = ~crc;
    for (i = 0; i < len; i++) {
        crc ^= p[i];
        for (b = 0; b < 8; b++) {
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
        }
    }
    return ~crc;
}

uint32_t efw_crc32(const void *data, size_t len)
{
    return efw_crc32_update(0, data, len);
}
