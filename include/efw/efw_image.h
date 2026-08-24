// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// ISO/IEC 25000 | ISO/IEC/IEEE 15288:2023

/**
 * @file efw_image.h
 * @brief eFirmware image container — build, serialise, parse, verify.
 *
 * The on-flash layout is byte-compatible with eBoot's eos_image_header_t
 * (eBoot/include/eos_image.h), so an image packed here is consumable by
 * eos_image_parse_header() without translation:
 *
 * @verbatim
 *   offset  size  field
 *   0       4     magic          EFW_IMAGE_MAGIC
 *   4       2     hdr_version    EFW_IMAGE_HDR_VERSION
 *   6       2     hdr_size       EFW_IMAGE_HDR_SIZE
 *   8       4     image_size     payload bytes, excluding this header
 *   12      4     load_addr
 *   16      4     entry_addr
 *   20      4     image_version  EFW_VERSION_MAKE(major, minor, patch)
 *   24      4     flags          EFW_IMG_FLAG_*
 *   28      32    hash           SHA-256 of the payload
 *   60      1     sig_type       efw_sig_type_t
 *   61      1     sig_len        bytes of `signature` in use
 *   62      30    reserved       must be zero
 *   92      64    signature
 *   156           total
 * @endverbatim
 *
 * Multi-byte fields are little-endian. That matches the in-memory struct
 * layout on every currently supported target (ARM and x86, both LE), which is
 * how eBoot reads the header straight out of flash.
 *
 * Signing is out of scope for this header: efw_image_build() populates
 * sig_type/sig_len/signature only when a caller supplies a signature, and
 * efw_image_verify() checks integrity, not authenticity. Callers that must
 * reject unsigned images are responsible for testing EFW_IMG_FLAG_SIGNED and
 * validating the signature with their own key material.
 */

#ifndef EFW_IMAGE_H
#define EFW_IMAGE_H

#include "efw/efw_sha256.h"
#include "efw/efw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Container magic, "EOSI" read as a big-endian word. Matches EOS_IMG_MAGIC. */
#define EFW_IMAGE_MAGIC 0x454F5349u

/** Header format revision this build reads and writes. */
#define EFW_IMAGE_HDR_VERSION 1u

/** Serialised header size in bytes. */
#define EFW_IMAGE_HDR_SIZE 156u

/** Bytes reserved for a signature. */
#define EFW_IMAGE_SIG_MAX 64u

/** Bytes of reserved padding between sig_len and signature. */
#define EFW_IMAGE_RESERVED 30u

/* Image flags, values shared with eBoot's EOS_IMG_FLAG_*. */
#define EFW_IMG_FLAG_ENCRYPTED   (1u << 0)
#define EFW_IMG_FLAG_COMPRESSED  (1u << 1)
#define EFW_IMG_FLAG_DEBUG       (1u << 2)
#define EFW_IMG_FLAG_RTOS        (1u << 3)
#define EFW_IMG_FLAG_LINUX       (1u << 4)
#define EFW_IMG_FLAG_SIGNED      (1u << 5)
#define EFW_IMG_FLAG_HASH_SHA256 (1u << 6)

/** Signature algorithm identifiers, shared with eBoot's eos_sig_type_t. */
typedef enum {
    EFW_SIG_NONE    = 0,
    EFW_SIG_CRC32   = 1,
    EFW_SIG_SHA256  = 2,
    EFW_SIG_ED25519 = 3,
    EFW_SIG_ECDSA   = 4
} efw_sig_type_t;

/** Parsed image header. Mirrors eBoot's eos_image_header_t field for field. */
typedef struct {
    uint32_t magic;
    uint16_t hdr_version;
    uint16_t hdr_size;
    uint32_t image_size;
    uint32_t load_addr;
    uint32_t entry_addr;
    uint32_t image_version;
    uint32_t flags;
    uint8_t  hash[EFW_SHA256_DIGEST_LEN];
    uint8_t  sig_type;
    uint8_t  sig_len;
    uint8_t  reserved[EFW_IMAGE_RESERVED];
    uint8_t  signature[EFW_IMAGE_SIG_MAX];
} efw_image_header_t;

/** Inputs to efw_image_build() that are not derived from the payload. */
typedef struct {
    uint32_t load_addr;
    uint32_t entry_addr;
    uint32_t image_version;  /**< Build with EFW_VERSION_MAKE(). */
    uint32_t flags;          /**< EFW_IMG_FLAG_*; HASH_SHA256 is added. */
} efw_image_params_t;

/**
 * @brief Populate a header for @p payload, hashing it with SHA-256.
 * @param params  Load/entry addresses, version and flags.
 * @param payload Image body; may be NULL only when @p payload_len is 0.
 * @param payload_len Length of @p payload in bytes.
 * @param out     Receives the completed header.
 * @return EFW_OK, or EFW_ERR_INVALID when @p params or @p out is NULL, or when
 *         @p payload is NULL with a non-zero length.
 *
 * Sets EFW_IMG_FLAG_HASH_SHA256 and leaves the signature fields zeroed; use
 * efw_image_attach_signature() to fill them.
 *
 * @code
 * efw_image_params_t p = {
 *     .load_addr = 0x08010000, .entry_addr = 0x08010100,
 *     .image_version = EFW_VERSION_MAKE(1, 2, 3), .flags = EFW_IMG_FLAG_RTOS,
 * };
 * efw_image_header_t h;
 * if (efw_image_build(&p, blob, blob_len, &h) == EFW_OK) { ... }
 * @endcode
 */
int efw_image_build(const efw_image_params_t *params,
                    const void *payload, size_t payload_len,
                    efw_image_header_t *out);

/**
 * @brief Attach a detached signature to a built header.
 * @param hdr      Header to modify.
 * @param sig_type Algorithm that produced @p sig.
 * @param sig      Signature bytes.
 * @param sig_len  Length of @p sig; must not exceed EFW_IMAGE_SIG_MAX.
 * @return EFW_OK, EFW_ERR_INVALID on NULL arguments, or EFW_ERR_FULL when
 *         @p sig_len exceeds EFW_IMAGE_SIG_MAX.
 *
 * Sets EFW_IMG_FLAG_SIGNED on success.
 */
int efw_image_attach_signature(efw_image_header_t *hdr,
                               efw_sig_type_t sig_type,
                               const uint8_t *sig, size_t sig_len);

/**
 * @brief Serialise a header to its little-endian on-flash form.
 * @param hdr Header to write.
 * @param out Receives exactly EFW_IMAGE_HDR_SIZE bytes.
 * @return EFW_OK, or EFW_ERR_INVALID on a NULL argument.
 */
int efw_image_serialize(const efw_image_header_t *hdr,
                        uint8_t out[EFW_IMAGE_HDR_SIZE]);

/**
 * @brief Parse a serialised header.
 * @param buf Bytes to read.
 * @param len Bytes available at @p buf.
 * @param out Receives the parsed header.
 * @return EFW_OK on success;
 *         EFW_ERR_INVALID on a NULL argument;
 *         EFW_ERR_NO_IMAGE when @p len is below EFW_IMAGE_HDR_SIZE or the
 *         magic does not match;
 *         EFW_ERR_VERSION when hdr_version is not EFW_IMAGE_HDR_VERSION;
 *         EFW_ERR_INVALID when hdr_size disagrees with EFW_IMAGE_HDR_SIZE or
 *         sig_len exceeds EFW_IMAGE_SIG_MAX.
 *
 * Validating hdr_size and sig_len here is what keeps a malformed or hostile
 * header from driving an over-long read in a caller that trusts those fields.
 */
int efw_image_parse(const uint8_t *buf, size_t len, efw_image_header_t *out);

/**
 * @brief Verify a payload against its header.
 * @param hdr         Parsed header.
 * @param payload     Image body; may be NULL only when the length is 0.
 * @param payload_len Bytes available at @p payload.
 * @return EFW_OK when the payload length and SHA-256 both match;
 *         EFW_ERR_INVALID on a NULL header or a NULL payload with non-zero
 *         length;
 *         EFW_ERR_NO_IMAGE when @p payload_len disagrees with
 *         hdr->image_size;
 *         EFW_ERR_CRC when the SHA-256 does not match.
 *
 * The digest comparison is constant-time.
 */
int efw_image_verify(const efw_image_header_t *hdr,
                     const void *payload, size_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /* EFW_IMAGE_H */
