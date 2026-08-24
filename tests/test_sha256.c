// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// SHA-256 conformance against the FIPS 180-4 / NIST CAVP example vectors.

#include "efw/efw_sha256.h"
#include "efw_test.h"

#include <stdlib.h>

static void hex(const uint8_t *d, size_t n, char *out)
{
    static const char *H = "0123456789abcdef";
    size_t i;
    for (i = 0; i < n; i++) {
        out[i * 2]     = H[(d[i] >> 4) & 0xF];
        out[i * 2 + 1] = H[d[i] & 0xF];
    }
    out[n * 2] = '\0';
}

static void expect_digest(const char *msg, size_t len, const char *want)
{
    uint8_t d[EFW_SHA256_DIGEST_LEN];
    char got[EFW_SHA256_DIGEST_LEN * 2 + 1];

    efw_sha256(msg, len, d);
    hex(d, sizeof(d), got);
    efw_tests_run++;
    if (strcmp(got, want) != 0) {
        efw_tests_failed++;
        printf("  [FAIL] digest mismatch\n    got  %s\n    want %s\n",
               got, want);
    }
}

int main(void)
{
    /* NIST published vectors. */
    expect_digest("", 0,
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    expect_digest("abc", 3,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    expect_digest("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56,
        "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    /* Length 55, 56 and 64 exercise the three padding paths: fits, spills into
     * an extra block, and lands exactly on a block boundary. */
    {
        char m[64];
        uint8_t a[EFW_SHA256_DIGEST_LEN], b[EFW_SHA256_DIGEST_LEN];
        size_t sizes[] = {55, 56, 63, 64};
        unsigned i;
        memset(m, 'x', sizeof(m));
        for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
            efw_sha256_ctx_t c;
            size_t n = sizes[i], j;
            efw_sha256(m, n, a);
            /* Same message, fed one byte at a time, must agree. */
            efw_sha256_init(&c);
            for (j = 0; j < n; j++) efw_sha256_update(&c, m + j, 1);
            efw_sha256_final(&c, b);
            EFW_CHECK_MEM_EQ(a, b, EFW_SHA256_DIGEST_LEN);
        }
    }

    /* One million 'a' — catches carry bugs in the 64-bit length field. */
    {
        efw_sha256_ctx_t c;
        uint8_t d[EFW_SHA256_DIGEST_LEN];
        char got[EFW_SHA256_DIGEST_LEN * 2 + 1];
        char *chunk = (char *)malloc(1000);
        int i;
        EFW_CHECK(chunk != NULL);
        if (chunk) {
            memset(chunk, 'a', 1000);
            efw_sha256_init(&c);
            for (i = 0; i < 1000; i++) efw_sha256_update(&c, chunk, 1000);
            efw_sha256_final(&c, d);
            hex(d, sizeof(d), got);
            EFW_CHECK(strcmp(got,
                "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc"
                "7112cd0") == 0);
            free(chunk);
        }
    }

    /* Constant-time comparison must still be correct. */
    {
        uint8_t x[EFW_SHA256_DIGEST_LEN], y[EFW_SHA256_DIGEST_LEN];
        memset(x, 0xAB, sizeof(x));
        memcpy(y, x, sizeof(y));
        EFW_CHECK(efw_sha256_equal(x, y) != 0);
        y[EFW_SHA256_DIGEST_LEN - 1] ^= 0x01u;   /* differ in the last byte */
        EFW_CHECK(efw_sha256_equal(x, y) == 0);
        y[EFW_SHA256_DIGEST_LEN - 1] ^= 0x01u;
        y[0] ^= 0x80u;                            /* differ in the first byte */
        EFW_CHECK(efw_sha256_equal(x, y) == 0);
        EFW_CHECK(efw_sha256_equal(NULL, y) == 0);
    }

    /* update() with a zero length must be a no-op, not a corruption. */
    {
        uint8_t a[EFW_SHA256_DIGEST_LEN], b[EFW_SHA256_DIGEST_LEN];
        efw_sha256_ctx_t c;
        efw_sha256("abc", 3, a);
        efw_sha256_init(&c);
        efw_sha256_update(&c, NULL, 0);
        efw_sha256_update(&c, "a", 1);
        efw_sha256_update(&c, "", 0);
        efw_sha256_update(&c, "bc", 2);
        efw_sha256_final(&c, b);
        EFW_CHECK_MEM_EQ(a, b, EFW_SHA256_DIGEST_LEN);
    }

    EFW_TEST_REPORT("sha256");
}
