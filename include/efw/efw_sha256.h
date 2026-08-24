// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_sha256.h
 * @brief SHA-256 (FIPS 180-4) used for firmware image integrity.
 *
 * Streaming interface so an image can be hashed without being held in RAM —
 * the constraint that matters on the device side, where images routinely
 * exceed available memory.
 */

#ifndef EFW_SHA256_H
#define EFW_SHA256_H

#include "efw/efw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Digest length in bytes. */
#define EFW_SHA256_DIGEST_LEN 32u

/** Input block length in bytes. */
#define EFW_SHA256_BLOCK_LEN 64u

/** Streaming hash state. Treat as opaque. */
typedef struct {
    uint32_t state[8];                    /**< Working chaining values.     */
    uint64_t bitlen;                      /**< Total message length in bits.*/
    uint8_t  buf[EFW_SHA256_BLOCK_LEN];   /**< Partial-block buffer.        */
    size_t   buf_len;                     /**< Bytes currently in buf.      */
} efw_sha256_ctx_t;

/**
 * @brief Begin a new digest.
 * @param ctx Context to initialise. Must not be NULL.
 */
void efw_sha256_init(efw_sha256_ctx_t *ctx);

/**
 * @brief Absorb more message bytes.
 * @param ctx  Initialised context.
 * @param data Bytes to absorb; may be NULL only when @p len is 0.
 * @param len  Number of bytes at @p data.
 */
void efw_sha256_update(efw_sha256_ctx_t *ctx, const void *data, size_t len);

/**
 * @brief Finish the digest and emit it.
 * @param ctx    Initialised context. Unusable afterwards until re-init.
 * @param digest Receives EFW_SHA256_DIGEST_LEN bytes.
 */
void efw_sha256_final(efw_sha256_ctx_t *ctx,
                      uint8_t digest[EFW_SHA256_DIGEST_LEN]);

/**
 * @brief One-shot convenience wrapper.
 * @param data   Message bytes; may be NULL only when @p len is 0.
 * @param len    Number of bytes at @p data.
 * @param digest Receives EFW_SHA256_DIGEST_LEN bytes.
 *
 * @code
 * uint8_t d[EFW_SHA256_DIGEST_LEN];
 * efw_sha256(         "abc", 3, d);
 * // d == ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad
 * @endcode
 */
void efw_sha256(const void *data, size_t len,
                uint8_t digest[EFW_SHA256_DIGEST_LEN]);

/**
 * @brief Constant-time digest comparison.
 * @param a First digest.
 * @param b Second digest.
 * @return Non-zero when equal. Runs in time independent of where they differ,
 *         so a caller verifying an image cannot be used as a byte oracle.
 */
int efw_sha256_equal(const uint8_t a[EFW_SHA256_DIGEST_LEN],
                     const uint8_t b[EFW_SHA256_DIGEST_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* EFW_SHA256_H */
