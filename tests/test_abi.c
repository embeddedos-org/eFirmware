// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Wire-format ABI: the on-disk layout eBoot parses independently.

/**
 * @file test_abi.c
 * @brief Pins the byte layout of efw_image_header_t.
 *
 * The .efw header is a wire format with two independent definitions:
 * `efw_image_header_t` here, and `eos_image_header_t` in eBoot
 * (`include/eos_image.h`). They declare the same magic — 0x454F5349, "EOSI" —
 * because eBoot parses the images this repository produces.
 *
 * Nothing checks that they agree. Both were written from the same table and
 * match today, verified by reading an efwtool-produced image with eBoot's
 * struct: 156 bytes, every field correct. But a field added, reordered or
 * widened on either side would produce a bootloader that silently misreads
 * images, and no test in either repository would fail.
 *
 * This pins the layout to the table in efw_image.h. It cannot see eBoot, so it
 * cannot prove the two still match — what it does is make one side unable to
 * drift silently. eBoot carries the mirror of this test.
 *
 * A failure here is not a bug in the test. It means the wire format changed,
 * and every consumer of it needs the same change plus a header-version bump.
 */

#include "efw/efw_image.h"
#include "efw_test.h"

#include <stddef.h>

/* offsetof against the documented table, field by field. A reorder that keeps
 * the total size — swapping two uint32_t — is invisible to a sizeof check and
 * is exactly the kind of change that corrupts a boot. */
static void test_field_offsets(void)
{
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, magic),         0);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, hdr_version),   4);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, hdr_size),      6);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, image_size),    8);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, load_addr),     12);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, entry_addr),    16);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, image_version), 20);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, flags),         24);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, hash),          28);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, sig_type),      60);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, sig_len),       61);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, reserved),      62);
    EFW_CHECK_EQ_INT(offsetof(efw_image_header_t, signature),     92);
}

static void test_total_size(void)
{
    /* Padding the compiler inserts is as much a wire-format change as a field
     * added by hand, and only sizeof catches it. */
    EFW_CHECK_EQ_INT(sizeof(efw_image_header_t), 156);
    EFW_CHECK_EQ_INT(EFW_IMAGE_HDR_SIZE,         156);
    EFW_CHECK_EQ_INT(sizeof(efw_image_header_t), EFW_IMAGE_HDR_SIZE);
}

static void test_field_widths(void)
{
    efw_image_header_t h;
    EFW_CHECK_EQ_INT(sizeof(h.magic),         4);
    EFW_CHECK_EQ_INT(sizeof(h.hdr_version),   2);
    EFW_CHECK_EQ_INT(sizeof(h.hdr_size),      2);
    EFW_CHECK_EQ_INT(sizeof(h.image_size),    4);
    EFW_CHECK_EQ_INT(sizeof(h.load_addr),     4);
    EFW_CHECK_EQ_INT(sizeof(h.entry_addr),    4);
    EFW_CHECK_EQ_INT(sizeof(h.image_version), 4);
    EFW_CHECK_EQ_INT(sizeof(h.flags),         4);
    EFW_CHECK_EQ_INT(sizeof(h.hash),          32);
    EFW_CHECK_EQ_INT(sizeof(h.reserved),      30);
    EFW_CHECK_EQ_INT(sizeof(h.signature),     64);
    EFW_CHECK_EQ_INT(EFW_IMAGE_SIG_MAX,       64);
}

static void test_shared_constants(void)
{
    /* eBoot's EOS_IMG_MAGIC carries the same value. If this changes, every
     * image already flashed becomes unreadable, so it changes only with a
     * hdr_version bump and a migration. */
    EFW_CHECK_EQ_INT(EFW_IMAGE_MAGIC, 0x454F5349u);   /* "EOSI" */
    EFW_CHECK_EQ_INT(EFW_IMAGE_HDR_VERSION, 1u);

    /* Signature identifiers are shared with eBoot's eos_sig_type_t; the
     * numbers travel in the image, so they are wire format too. */
    EFW_CHECK_EQ_INT(EFW_SIG_NONE,    0);
    EFW_CHECK_EQ_INT(EFW_SIG_CRC32,   1);
    EFW_CHECK_EQ_INT(EFW_SIG_SHA256,  2);
    EFW_CHECK_EQ_INT(EFW_SIG_ED25519, 3);
    EFW_CHECK_EQ_INT(EFW_SIG_ECDSA,   4);
}

static void test_serialized_bytes_match_the_struct(void)
{
    /* offsetof describes the struct. This describes what reaches the disk,
     * which is what eBoot actually reads. */
    static const uint8_t payload[4] = { 1, 2, 3, 4 };
    efw_image_params_t p;
    efw_image_header_t h;
    uint8_t buf[EFW_IMAGE_HDR_SIZE];

    memset(&p, 0, sizeof(p));
    p.load_addr  = 0x08000000u;
    p.entry_addr = 0x08000100u;
    p.flags      = EFW_IMG_FLAG_RTOS;

    EFW_CHECK_EQ_INT(efw_image_build(&p, payload, sizeof(payload), &h), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_serialize(&h, buf), EFW_OK);

    /* Little-endian magic at offset 0 — the first thing any reader checks. */
    EFW_CHECK_EQ_INT(buf[0], 0x49);
    EFW_CHECK_EQ_INT(buf[1], 0x53);
    EFW_CHECK_EQ_INT(buf[2], 0x4F);
    EFW_CHECK_EQ_INT(buf[3], 0x45);
    EFW_CHECK_EQ_INT(buf[6], 156);      /* hdr_size, low byte */
    EFW_CHECK_EQ_INT(buf[8], 4);        /* image_size, low byte */
    EFW_CHECK_EQ_INT(buf[60], EFW_SIG_NONE);
    EFW_CHECK_EQ_INT(buf[61], 0);       /* sig_len */

    /* reserved must be zero: a consumer is entitled to reject a non-zero
     * value as an image from a newer format it cannot read. */
    for (size_t i = 62; i < 92; i++) {
        EFW_CHECK_EQ_INT(buf[i], 0);
    }
}

int main(void)
{
    test_field_offsets();
    test_total_size();
    test_field_widths();
    test_shared_constants();
    test_serialized_bytes_match_the_struct();
    EFW_TEST_REPORT("abi");
}
