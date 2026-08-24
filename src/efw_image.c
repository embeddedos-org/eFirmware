// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_image.c
 * @brief eFirmware image container implementation.
 */

#include "efw/efw_image.h"

#include <string.h>

/* ---- little-endian helpers ------------------------------------------------
 * Written byte-at-a-time rather than by casting a struct pointer: the on-flash
 * format is fixed, and a memcpy of the struct would silently inherit whatever
 * padding and byte order the host compiler chose. */

static void put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint16_t get_u16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* ---- API ---------------------------------------------------------------- */

int efw_image_build(const efw_image_params_t *params,
                    const void *payload, size_t payload_len,
                    efw_image_header_t *out)
{
    if (!params || !out) return EFW_ERR_INVALID;
    if (!payload && payload_len > 0) return EFW_ERR_INVALID;
    if (payload_len > 0xFFFFFFFFu) return EFW_ERR_INVALID;

    memset(out, 0, sizeof(*out));
    out->magic         = EFW_IMAGE_MAGIC;
    out->hdr_version   = (uint16_t)EFW_IMAGE_HDR_VERSION;
    out->hdr_size      = (uint16_t)EFW_IMAGE_HDR_SIZE;
    out->image_size    = (uint32_t)payload_len;
    out->load_addr     = params->load_addr;
    out->entry_addr    = params->entry_addr;
    out->image_version = params->image_version;
    out->flags         = params->flags | EFW_IMG_FLAG_HASH_SHA256;
    out->sig_type      = (uint8_t)EFW_SIG_NONE;
    out->sig_len       = 0;

    efw_sha256(payload, payload_len, out->hash);
    return EFW_OK;
}

int efw_image_attach_signature(efw_image_header_t *hdr,
                               efw_sig_type_t sig_type,
                               const uint8_t *sig, size_t sig_len)
{
    if (!hdr || !sig) return EFW_ERR_INVALID;
    if (sig_len == 0) return EFW_ERR_INVALID;
    if (sig_len > EFW_IMAGE_SIG_MAX) return EFW_ERR_FULL;

    memset(hdr->signature, 0, sizeof(hdr->signature));
    memcpy(hdr->signature, sig, sig_len);
    hdr->sig_type = (uint8_t)sig_type;
    hdr->sig_len  = (uint8_t)sig_len;
    hdr->flags   |= EFW_IMG_FLAG_SIGNED;
    return EFW_OK;
}

int efw_image_serialize(const efw_image_header_t *hdr,
                        uint8_t out[EFW_IMAGE_HDR_SIZE])
{
    if (!hdr || !out) return EFW_ERR_INVALID;

    memset(out, 0, EFW_IMAGE_HDR_SIZE);
    put_u32(out + 0,  hdr->magic);
    put_u16(out + 4,  hdr->hdr_version);
    put_u16(out + 6,  hdr->hdr_size);
    put_u32(out + 8,  hdr->image_size);
    put_u32(out + 12, hdr->load_addr);
    put_u32(out + 16, hdr->entry_addr);
    put_u32(out + 20, hdr->image_version);
    put_u32(out + 24, hdr->flags);
    memcpy(out + 28, hdr->hash, EFW_SHA256_DIGEST_LEN);
    out[60] = hdr->sig_type;
    out[61] = hdr->sig_len;
    /* bytes 62..91 stay zero: `reserved` is required to be zero on the wire */
    memcpy(out + 92, hdr->signature, EFW_IMAGE_SIG_MAX);
    return EFW_OK;
}

int efw_image_parse(const uint8_t *buf, size_t len, efw_image_header_t *out)
{
    efw_image_header_t h;

    if (!buf || !out) return EFW_ERR_INVALID;
    if (len < EFW_IMAGE_HDR_SIZE) return EFW_ERR_NO_IMAGE;

    memset(&h, 0, sizeof(h));
    h.magic = get_u32(buf + 0);
    if (h.magic != EFW_IMAGE_MAGIC) return EFW_ERR_NO_IMAGE;

    h.hdr_version = get_u16(buf + 4);
    if (h.hdr_version != EFW_IMAGE_HDR_VERSION) return EFW_ERR_VERSION;

    h.hdr_size = get_u16(buf + 6);
    if (h.hdr_size != (uint16_t)EFW_IMAGE_HDR_SIZE) return EFW_ERR_INVALID;

    h.image_size    = get_u32(buf + 8);
    h.load_addr     = get_u32(buf + 12);
    h.entry_addr    = get_u32(buf + 16);
    h.image_version = get_u32(buf + 20);
    h.flags         = get_u32(buf + 24);
    memcpy(h.hash, buf + 28, EFW_SHA256_DIGEST_LEN);
    h.sig_type = buf[60];
    h.sig_len  = buf[61];

    /* Reject before a caller can use sig_len to size a read. */
    if (h.sig_len > EFW_IMAGE_SIG_MAX) return EFW_ERR_INVALID;

    memcpy(h.reserved, buf + 62, EFW_IMAGE_RESERVED);
    memcpy(h.signature, buf + 92, EFW_IMAGE_SIG_MAX);

    *out = h;
    return EFW_OK;
}

int efw_image_verify(const efw_image_header_t *hdr,
                     const void *payload, size_t payload_len)
{
    uint8_t digest[EFW_SHA256_DIGEST_LEN];

    if (!hdr) return EFW_ERR_INVALID;
    if (!payload && payload_len > 0) return EFW_ERR_INVALID;
    if (payload_len != (size_t)hdr->image_size) return EFW_ERR_NO_IMAGE;

    efw_sha256(payload, payload_len, digest);
    if (!efw_sha256_equal(digest, hdr->hash)) return EFW_ERR_CRC;

    return EFW_OK;
}

const char *efw_status_str(int status)
{
    switch (status) {
    case EFW_OK:                 return "OK";
    case EFW_ERR_GENERIC:        return "ERR_GENERIC";
    case EFW_ERR_INVALID:        return "ERR_INVALID";
    case EFW_ERR_CRC:            return "ERR_CRC";
    case EFW_ERR_SIGNATURE:      return "ERR_SIGNATURE";
    case EFW_ERR_NO_IMAGE:       return "ERR_NO_IMAGE";
    case EFW_ERR_FLASH:          return "ERR_FLASH";
    case EFW_ERR_TIMEOUT:        return "ERR_TIMEOUT";
    case EFW_ERR_BUSY:           return "ERR_BUSY";
    case EFW_ERR_AUTH:           return "ERR_AUTH";
    case EFW_ERR_VERSION:        return "ERR_VERSION";
    case EFW_ERR_FULL:           return "ERR_FULL";
    case EFW_ERR_NOT_FOUND:      return "ERR_NOT_FOUND";
    case EFW_ERR_NOT_SUPPORTED:  return "ERR_NOT_SUPPORTED";
    case EFW_ERR_ANTI_ROLLBACK:  return "ERR_ANTI_ROLLBACK";
    case EFW_ERR_REPLAY:         return "ERR_REPLAY";
    case EFW_ERR_DECRYPT:        return "ERR_DECRYPT";
    case EFW_ERR_KEY:            return "ERR_KEY";
    case EFW_ERR_DEBUG_LOCKED:   return "ERR_DEBUG_LOCKED";
    default:                     return "ERR_UNKNOWN";
    }
}
