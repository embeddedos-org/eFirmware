// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Image container: round-trip, integrity, and malformed-header rejection.

#include "efw/efw_image.h"
#include "efw_test.h"

static const uint8_t PAYLOAD[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB
};

static efw_image_params_t sample_params(void)
{
    efw_image_params_t p;
    memset(&p, 0, sizeof(p));
    p.load_addr     = 0x08010000u;
    p.entry_addr    = 0x08010100u;
    p.image_version = EFW_VERSION_MAKE(1, 2, 300);
    p.flags         = EFW_IMG_FLAG_RTOS;
    return p;
}

int main(void)
{
    efw_image_params_t p = sample_params();
    efw_image_header_t h, r;
    uint8_t wire[EFW_IMAGE_HDR_SIZE];

    /* ---- build ---------------------------------------------------------- */
    EFW_CHECK_EQ_INT(efw_image_build(&p, PAYLOAD, sizeof(PAYLOAD), &h), EFW_OK);
    EFW_CHECK(h.magic == EFW_IMAGE_MAGIC);
    EFW_CHECK_EQ_INT(h.hdr_version, EFW_IMAGE_HDR_VERSION);
    EFW_CHECK_EQ_INT(h.hdr_size, (int)EFW_IMAGE_HDR_SIZE);
    EFW_CHECK_EQ_INT(h.image_size, (int)sizeof(PAYLOAD));
    EFW_CHECK(h.load_addr == 0x08010000u);
    EFW_CHECK(h.entry_addr == 0x08010100u);
    /* build() must add the hash flag without dropping the caller's flags */
    EFW_CHECK((h.flags & EFW_IMG_FLAG_RTOS) != 0);
    EFW_CHECK((h.flags & EFW_IMG_FLAG_HASH_SHA256) != 0);
    EFW_CHECK((h.flags & EFW_IMG_FLAG_SIGNED) == 0);
    EFW_CHECK_EQ_INT(h.sig_type, EFW_SIG_NONE);
    EFW_CHECK_EQ_INT(h.sig_len, 0);

    /* version round-trips through the packing macros */
    EFW_CHECK_EQ_INT(EFW_VERSION_MAJOR(h.image_version), 1);
    EFW_CHECK_EQ_INT(EFW_VERSION_MINOR(h.image_version), 2);
    EFW_CHECK_EQ_INT(EFW_VERSION_PATCH(h.image_version), 300);

    /* ---- build rejects bad arguments ------------------------------------ */
    EFW_CHECK_EQ_INT(efw_image_build(NULL, PAYLOAD, sizeof(PAYLOAD), &h),
                     EFW_ERR_INVALID);
    EFW_CHECK_EQ_INT(efw_image_build(&p, PAYLOAD, sizeof(PAYLOAD), NULL),
                     EFW_ERR_INVALID);
    EFW_CHECK_EQ_INT(efw_image_build(&p, NULL, 8, &h), EFW_ERR_INVALID);

    /* a zero-length payload is legal and hashes the empty string */
    {
        efw_image_header_t z;
        uint8_t empty_digest[EFW_SHA256_DIGEST_LEN];
        EFW_CHECK_EQ_INT(efw_image_build(&p, NULL, 0, &z), EFW_OK);
        efw_sha256("", 0, empty_digest);
        EFW_CHECK_MEM_EQ(z.hash, empty_digest, EFW_SHA256_DIGEST_LEN);
        EFW_CHECK_EQ_INT(efw_image_verify(&z, NULL, 0), EFW_OK);
    }

    /* ---- serialise / parse round-trip ----------------------------------- */
    EFW_CHECK_EQ_INT(efw_image_build(&p, PAYLOAD, sizeof(PAYLOAD), &h), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_serialize(&h, wire), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_parse(wire, sizeof(wire), &r), EFW_OK);
    EFW_CHECK(r.magic == h.magic);
    EFW_CHECK_EQ_INT(r.image_size, (int)h.image_size);
    EFW_CHECK(r.load_addr == h.load_addr);
    EFW_CHECK(r.entry_addr == h.entry_addr);
    EFW_CHECK(r.image_version == h.image_version);
    EFW_CHECK(r.flags == h.flags);
    EFW_CHECK_MEM_EQ(r.hash, h.hash, EFW_SHA256_DIGEST_LEN);

    /* the documented byte offsets are part of the contract with eBoot */
    EFW_CHECK_EQ_INT(wire[0], 0x49);  /* magic, little-endian: 49 53 4F 45 */
    EFW_CHECK_EQ_INT(wire[1], 0x53);
    EFW_CHECK_EQ_INT(wire[2], 0x4F);
    EFW_CHECK_EQ_INT(wire[3], 0x45);
    EFW_CHECK_EQ_INT(wire[6], (int)(EFW_IMAGE_HDR_SIZE & 0xFF));
    /* `reserved` must be zero on the wire */
    {
        unsigned i, nonzero = 0;
        for (i = 62; i < 92; i++) if (wire[i] != 0) nonzero++;
        EFW_CHECK_EQ_INT(nonzero, 0);
    }

    /* ---- verify --------------------------------------------------------- */
    EFW_CHECK_EQ_INT(efw_image_verify(&h, PAYLOAD, sizeof(PAYLOAD)), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_verify(NULL, PAYLOAD, sizeof(PAYLOAD)),
                     EFW_ERR_INVALID);
    EFW_CHECK_EQ_INT(efw_image_verify(&h, NULL, sizeof(PAYLOAD)),
                     EFW_ERR_INVALID);
    /* a truncated or padded payload is a length mismatch, not a hash failure */
    EFW_CHECK_EQ_INT(efw_image_verify(&h, PAYLOAD, sizeof(PAYLOAD) - 1),
                     EFW_ERR_NO_IMAGE);

    /* every single-bit flip in the payload must be detected */
    {
        uint8_t tampered[sizeof(PAYLOAD)];
        size_t byte;
        unsigned bit;
        for (byte = 0; byte < sizeof(PAYLOAD); byte++) {
            for (bit = 0; bit < 8; bit++) {
                memcpy(tampered, PAYLOAD, sizeof(tampered));
                tampered[byte] ^= (uint8_t)(1u << bit);
                EFW_CHECK_EQ_INT(
                    efw_image_verify(&h, tampered, sizeof(tampered)),
                    EFW_ERR_CRC);
            }
        }
    }

    /* ---- malformed headers are rejected --------------------------------- */
    EFW_CHECK_EQ_INT(efw_image_parse(NULL, sizeof(wire), &r), EFW_ERR_INVALID);
    EFW_CHECK_EQ_INT(efw_image_parse(wire, sizeof(wire), NULL), EFW_ERR_INVALID);
    /* one byte short of a header */
    EFW_CHECK_EQ_INT(efw_image_parse(wire, EFW_IMAGE_HDR_SIZE - 1, &r),
                     EFW_ERR_NO_IMAGE);

    {
        uint8_t bad[EFW_IMAGE_HDR_SIZE];

        memcpy(bad, wire, sizeof(bad));
        bad[0] ^= 0xFFu;                                  /* wrong magic */
        EFW_CHECK_EQ_INT(efw_image_parse(bad, sizeof(bad), &r),
                         EFW_ERR_NO_IMAGE);

        memcpy(bad, wire, sizeof(bad));
        bad[4] = 0x99; bad[5] = 0x00;                     /* unknown version */
        EFW_CHECK_EQ_INT(efw_image_parse(bad, sizeof(bad), &r),
                         EFW_ERR_VERSION);

        memcpy(bad, wire, sizeof(bad));
        bad[6] = 0xFF; bad[7] = 0xFF;                     /* bogus hdr_size */
        EFW_CHECK_EQ_INT(efw_image_parse(bad, sizeof(bad), &r),
                         EFW_ERR_INVALID);

        memcpy(bad, wire, sizeof(bad));
        bad[61] = (uint8_t)(EFW_IMAGE_SIG_MAX + 1);       /* oversized sig_len */
        EFW_CHECK_EQ_INT(efw_image_parse(bad, sizeof(bad), &r),
                         EFW_ERR_INVALID);

        memcpy(bad, wire, sizeof(bad));
        bad[61] = (uint8_t)EFW_IMAGE_SIG_MAX;             /* exactly at limit */
        EFW_CHECK_EQ_INT(efw_image_parse(bad, sizeof(bad), &r), EFW_OK);
    }

    /* ---- signature attachment ------------------------------------------- */
    {
        uint8_t sig[EFW_IMAGE_SIG_MAX];
        uint8_t over[EFW_IMAGE_SIG_MAX + 1];
        memset(sig, 0xC3, sizeof(sig));
        memset(over, 0x11, sizeof(over));

        EFW_CHECK_EQ_INT(efw_image_build(&p, PAYLOAD, sizeof(PAYLOAD), &h),
                         EFW_OK);
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(&h, EFW_SIG_ED25519, sig, sizeof(sig)),
            EFW_OK);
        EFW_CHECK_EQ_INT(h.sig_type, EFW_SIG_ED25519);
        EFW_CHECK_EQ_INT(h.sig_len, (int)EFW_IMAGE_SIG_MAX);
        EFW_CHECK((h.flags & EFW_IMG_FLAG_SIGNED) != 0);
        EFW_CHECK_MEM_EQ(h.signature, sig, sizeof(sig));

        /* over-long signature must be refused, leaving the header untouched */
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(&h, EFW_SIG_ED25519, over, sizeof(over)),
            EFW_ERR_FULL);
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(NULL, EFW_SIG_ED25519, sig, 4),
            EFW_ERR_INVALID);
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(&h, EFW_SIG_ED25519, NULL, 4),
            EFW_ERR_INVALID);
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(&h, EFW_SIG_ED25519, sig, 0),
            EFW_ERR_INVALID);

        /* a short signature must be zero-padded, not leave stale bytes */
        EFW_CHECK_EQ_INT(
            efw_image_attach_signature(&h, EFW_SIG_ECDSA, sig, 8), EFW_OK);
        EFW_CHECK_EQ_INT(h.sig_len, 8);
        {
            unsigned i, nonzero = 0;
            for (i = 8; i < EFW_IMAGE_SIG_MAX; i++)
                if (h.signature[i] != 0) nonzero++;
            EFW_CHECK_EQ_INT(nonzero, 0);
        }

        /* signature round-trips through the wire form */
        EFW_CHECK_EQ_INT(efw_image_serialize(&h, wire), EFW_OK);
        EFW_CHECK_EQ_INT(efw_image_parse(wire, sizeof(wire), &r), EFW_OK);
        EFW_CHECK_EQ_INT(r.sig_len, 8);
        EFW_CHECK_EQ_INT(r.sig_type, EFW_SIG_ECDSA);
        EFW_CHECK_MEM_EQ(r.signature, h.signature, EFW_IMAGE_SIG_MAX);
    }

    /* ---- status strings ------------------------------------------------- */
    EFW_CHECK(strcmp(efw_status_str(EFW_OK), "OK") == 0);
    EFW_CHECK(strcmp(efw_status_str(EFW_ERR_CRC), "ERR_CRC") == 0);
    EFW_CHECK(strcmp(efw_status_str(EFW_ERR_NO_IMAGE), "ERR_NO_IMAGE") == 0);
    EFW_CHECK(strcmp(efw_status_str(12345), "ERR_UNKNOWN") == 0);

    EFW_TEST_REPORT("image");
}
