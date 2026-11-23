// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// examples/stm32f4/verify_stm32f4.c, built on the host with its memory-mapped
// "flash" pointed at a buffer this test owns. The example's own addresses are
// real STM32F4 addresses, so the macros are overridden here rather than in the
// example: a device build still gets 0x08000000 / 0x00010000 / 448 KiB.

#include "efw/efw_image.h"
#include "efw_test.h"

#include <stdint.h>
#include <string.h>

/* The slot the example will treat as application flash. Declared before the
 * example is included, because the macros below name it. */
#define SLOT_LEN 4096u
static uint8_t flash_slot[SLOT_LEN];

#define STM32F4_FLASH_BASE  ((uintptr_t)flash_slot)
#define STM32F4_APP_OFFSET  0u
#define STM32F4_APP_MAX_LEN ((size_t)SLOT_LEN)

int verify_stm32f4_image(void);

#include "../examples/stm32f4/verify_stm32f4.c"

/* ---- helpers -------------------------------------------------------------- */

static void write_image(const uint8_t *payload, size_t len)
{
    efw_image_params_t p;
    efw_image_header_t h;

    memset(&p, 0, sizeof(p));
    p.load_addr    = 0x08010000u;
    p.entry_addr   = 0x08010100u;
    p.image_version = EFW_VERSION_MAKE(1, 0, 0);

    memset(flash_slot, 0xFF, sizeof(flash_slot));
    EFW_CHECK_EQ_INT(efw_image_build(&p, payload, len, &h), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_serialize(&h, flash_slot), EFW_OK);
    memcpy(flash_slot + EFW_IMAGE_HDR_SIZE, payload, len);
}

/* Overwrite the serialised image_size field (offset 8, little-endian) without
 * touching the digest, which is how a hostile or corrupt image reaches the
 * length check with a size the slot cannot hold. */
static void poke_image_size(uint32_t v)
{
    flash_slot[8]  = (uint8_t)(v & 0xFFu);
    flash_slot[9]  = (uint8_t)((v >> 8) & 0xFFu);
    flash_slot[10] = (uint8_t)((v >> 16) & 0xFFu);
    flash_slot[11] = (uint8_t)((v >> 24) & 0xFFu);
}

/* ---- test ----------------------------------------------------------------- */

int main(void)
{
    static const uint8_t payload[] = "eFirmware STM32F4 example payload";

    /* An intact image in the slot verifies. */
    write_image(payload, sizeof(payload));
    EFW_CHECK_EQ_INT(verify_stm32f4_image(), 0);

    /* A tampered payload is refused. */
    flash_slot[EFW_IMAGE_HDR_SIZE] = (uint8_t)(flash_slot[EFW_IMAGE_HDR_SIZE] ^ 0x01u);
    EFW_CHECK_EQ_INT(verify_stm32f4_image(), -1);

    /* Erased flash has no header and is refused. */
    memset(flash_slot, 0xFF, sizeof(flash_slot));
    EFW_CHECK_EQ_INT(verify_stm32f4_image(), -1);

    /* An image_size larger than the slot is refused before the payload is
     * hashed. UINT32_MAX is the case the old `EFW_IMAGE_HDR_SIZE + payload_len`
     * form let through on a 32-bit target, where that sum wraps to 155. */
    write_image(payload, sizeof(payload));
    poke_image_size(0xFFFFFFFFu);
    EFW_CHECK_EQ_INT(verify_stm32f4_image(), -1);

    /* The boundary either side of the slot: exactly the space left after the
     * header is still refused here only because the digest no longer matches,
     * while one byte more must be refused by the length check itself. */
    write_image(payload, sizeof(payload));
    poke_image_size((uint32_t)(SLOT_LEN - EFW_IMAGE_HDR_SIZE + 1u));
    EFW_CHECK_EQ_INT(verify_stm32f4_image(), -1);

    /* Why the check had to change shape rather than just gain a case.
     *
     * On this host size_t is 64-bit, so the two cases above are refused with
     * either form of the check and cannot, by themselves, show the defect.
     * The target this example is written for (STM32F4, Cortex-M4) has a
     * 32-bit size_t, and `EFW_IMAGE_HDR_SIZE + payload_len` is then computed
     * in 32-bit arithmetic. Reproduced below in uint32_t, which every host
     * evaluates identically: the old sum wraps to 155 and compares as "fits",
     * while the subtraction form rejects the same input. */
    {
        const uint32_t hdr  = (uint32_t)EFW_IMAGE_HDR_SIZE;
        const uint32_t len  = 0xFFFFFFFFu;
        const uint32_t slot = (uint32_t)SLOT_LEN;

        EFW_CHECK_EQ_INT((long)(hdr + len), 155L);   /* the wrap itself */
        EFW_CHECK(!(hdr + len > slot));              /* old form: accepts */
        EFW_CHECK(len > slot - hdr);                 /* new form: refuses */
    }

    EFW_TEST_REPORT("example_stm32f4");
}
