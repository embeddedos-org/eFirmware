// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_crc32.h
 * @brief CRC-32/ISO-HDLC, the variant eBoot reports as EOS_ERR_CRC on mismatch.
 *
 * Parameters: poly 0x04C11DB7 reflected (0xEDB88320), init 0xFFFFFFFF,
 * reflect in/out, xorout 0xFFFFFFFF. Same function as zlib's crc32().
 */

#ifndef EFW_CRC32_H
#define EFW_CRC32_H

#include "efw/efw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Continue a CRC over another chunk.
 * @param crc  Running value; pass 0 for the first chunk.
 * @param data Bytes to fold in; may be NULL only when @p len is 0.
 * @param len  Number of bytes at @p data.
 * @return The updated CRC, suitable to pass back in for the next chunk.
 *
 * @code
 * uint32_t c = efw_crc32_update(0, "1234", 4);
 * c = efw_crc32_update(c, "56789", 5);   // c == 0xCBF43926
 * @endcode
 */
uint32_t efw_crc32_update(uint32_t crc, const void *data, size_t len);

/**
 * @brief One-shot CRC.
 * @param data Bytes to hash; may be NULL only when @p len is 0.
 * @param len  Number of bytes at @p data.
 * @return CRC-32/ISO-HDLC of the buffer.
 */
uint32_t efw_crc32(const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* EFW_CRC32_H */
