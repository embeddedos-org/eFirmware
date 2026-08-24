// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// CRC-32/ISO-HDLC conformance and chunk-boundary independence.

#include "efw/efw_crc32.h"
#include "efw_test.h"

int main(void)
{
    /* Published check values for this CRC variant. */
    EFW_CHECK(efw_crc32("123456789", 9) == 0xCBF43926u);
    EFW_CHECK(efw_crc32("", 0) == 0x00000000u);
    EFW_CHECK(efw_crc32("a", 1) == 0xE8B7BE43u);
    EFW_CHECK(efw_crc32("abc", 3) == 0x352441C2u);

    /* Splitting the input must not change the result, at every boundary. */
    {
        const char *m = "123456789";
        size_t split;
        for (split = 0; split <= 9; split++) {
            uint32_t c = efw_crc32_update(0, m, split);
            c = efw_crc32_update(c, m + split, 9 - split);
            EFW_CHECK(c == 0xCBF43926u);
        }
    }

    /* A NULL buffer with zero length is a no-op, not a crash. */
    EFW_CHECK(efw_crc32_update(0x1234u, NULL, 0) == 0x1234u);
    EFW_CHECK(efw_crc32(NULL, 0) == 0u);

    /* A single flipped bit must change the CRC. */
    {
        uint8_t a[32], b[32];
        memset(a, 0x5A, sizeof(a));
        memcpy(b, a, sizeof(b));
        b[17] ^= 0x01u;
        EFW_CHECK(efw_crc32(a, sizeof(a)) != efw_crc32(b, sizeof(b)));
    }

    EFW_TEST_REPORT("crc32");
}
