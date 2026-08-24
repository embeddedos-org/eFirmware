// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_sha256.c
 * @brief SHA-256 per FIPS 180-4, section 6.2.
 */

#include "efw/efw_sha256.h"

#include <string.h>

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/* First 32 bits of the fractional parts of the cube roots of the first
 * 64 primes (FIPS 180-4, section 4.2.2). */
static const uint32_t K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static void sha256_compress(uint32_t state[8], const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    unsigned i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (i = 0; i < 64; i++) {
        uint32_t t1 = h + BSIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void efw_sha256_init(efw_sha256_ctx_t *ctx)
{
    if (!ctx) return;
    /* First 32 bits of the fractional parts of the square roots of the
     * first eight primes (FIPS 180-4, section 5.3.3). */
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->bitlen = 0;
    ctx->buf_len = 0;
    memset(ctx->buf, 0, sizeof(ctx->buf));
}

void efw_sha256_update(efw_sha256_ctx_t *ctx, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;

    if (!ctx || len == 0) return;
    if (!p) return;

    ctx->bitlen += (uint64_t)len * 8u;

    /* Top off any partial block first. */
    if (ctx->buf_len > 0) {
        size_t need = EFW_SHA256_BLOCK_LEN - ctx->buf_len;
        size_t take = (len < need) ? len : need;
        memcpy(ctx->buf + ctx->buf_len, p, take);
        ctx->buf_len += take;
        p += take;
        len -= take;
        if (ctx->buf_len < EFW_SHA256_BLOCK_LEN) return;
        sha256_compress(ctx->state, ctx->buf);
        ctx->buf_len = 0;
    }

    while (len >= EFW_SHA256_BLOCK_LEN) {
        sha256_compress(ctx->state, p);
        p += EFW_SHA256_BLOCK_LEN;
        len -= EFW_SHA256_BLOCK_LEN;
    }

    if (len > 0) {
        memcpy(ctx->buf, p, len);
        ctx->buf_len = len;
    }
}

void efw_sha256_final(efw_sha256_ctx_t *ctx,
                      uint8_t digest[EFW_SHA256_DIGEST_LEN])
{
    uint64_t bitlen;
    unsigned i;

    if (!ctx || !digest) return;

    bitlen = ctx->bitlen;

    /* Append 0x80, then zeros, leaving room for the 8-byte length. */
    ctx->buf[ctx->buf_len++] = 0x80u;
    if (ctx->buf_len > EFW_SHA256_BLOCK_LEN - 8u) {
        memset(ctx->buf + ctx->buf_len, 0, EFW_SHA256_BLOCK_LEN - ctx->buf_len);
        sha256_compress(ctx->state, ctx->buf);
        ctx->buf_len = 0;
    }
    memset(ctx->buf + ctx->buf_len,
           0, (EFW_SHA256_BLOCK_LEN - 8u) - ctx->buf_len);

    for (i = 0; i < 8; i++) {
        ctx->buf[EFW_SHA256_BLOCK_LEN - 1u - i] =
            (uint8_t)((bitlen >> (8u * i)) & 0xFFu);
    }
    sha256_compress(ctx->state, ctx->buf);

    for (i = 0; i < 8; i++) {
        digest[i * 4]     = (uint8_t)((ctx->state[i] >> 24) & 0xFFu);
        digest[i * 4 + 1] = (uint8_t)((ctx->state[i] >> 16) & 0xFFu);
        digest[i * 4 + 2] = (uint8_t)((ctx->state[i] >> 8) & 0xFFu);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i] & 0xFFu);
    }

    /* Do not leave chaining state or buffered plaintext on the stack/heap. */
    memset(ctx, 0, sizeof(*ctx));
}

void efw_sha256(const void *data, size_t len,
                uint8_t digest[EFW_SHA256_DIGEST_LEN])
{
    efw_sha256_ctx_t ctx;
    efw_sha256_init(&ctx);
    efw_sha256_update(&ctx, data, len);
    efw_sha256_final(&ctx, digest);
}

int efw_sha256_equal(const uint8_t a[EFW_SHA256_DIGEST_LEN],
                     const uint8_t b[EFW_SHA256_DIGEST_LEN])
{
    uint8_t diff = 0;
    unsigned i;

    if (!a || !b) return 0;
    for (i = 0; i < EFW_SHA256_DIGEST_LEN; i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}
