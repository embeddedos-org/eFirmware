// SPDX-License-Identifier: MIT
// Device-side verify snippet for STM32F4 (bare-metal).
//
// Reads the eFirmware image back from the application flash slot, parses
// the header and verifies the payload digest before eBoot boots it.
// The STM32F4 maps flash directly into the address space at 0x08000000,
// so no vendor flash API is needed -- flash_map() is just a pointer.
// The eFirmware calls are platform-independent.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "efw/efw_image.h"

/* --- platform glue (STM32F4 memory-mapped flash) ---------------------- */
#define STM32F4_FLASH_BASE  0x08000000u
#define STM32F4_APP_OFFSET  0x00010000u  /* first 64 KiB reserved for eBoot */
#define STM32F4_APP_MAX_LEN 0x00070000u  /* app slot: 448 KiB (512 KiB part) */

static const uint8_t *flash_map(size_t *out_len) {
    *out_len = STM32F4_APP_MAX_LEN;
    return (const uint8_t *)(STM32F4_FLASH_BASE + STM32F4_APP_OFFSET);
}

static void flash_unmap(const uint8_t *ptr) {
    (void)ptr;  /* nothing to release: flash is memory-mapped */
}
/* --- end platform glue ------------------------------------------------- */

// Returns 0 when the image in flash is intact and bootable, <0 otherwise.
int verify_stm32f4_image(void) {
    size_t flash_len = 0;
    const uint8_t *flash = flash_map(&flash_len);
    if (!flash) {
        printf("efw: no application flash slot\n");
        return -1;
    }

    efw_image_header_t hdr;
    // Parse clamps hdr_size/sig_len, so a hostile header cannot drive an
    // over-long read below.
    if (efw_image_parse(flash, flash_len, &hdr) != 0) {
        printf("efw: bad image header\n");
        flash_unmap(flash);
        return -1;
    }

    const uint8_t *payload = flash + EFW_IMAGE_HDR_SIZE;
    size_t payload_len = (size_t)hdr.image_size;
    if (EFW_IMAGE_HDR_SIZE + payload_len > flash_len) {
        printf("efw: image overruns application slot\n");
        flash_unmap(flash);
        return -1;
    }

    // Constant-time SHA-256 comparison against the header digest.
    if (efw_image_verify(&hdr, payload, payload_len) != 0) {
        printf("efw: payload digest mismatch -- refusing to boot\n");
        flash_unmap(flash);
        return -1;
    }

    printf("efw: image v%u.%u.%u verified, entry 0x%08x\n",
           (unsigned)EFW_VERSION_MAJOR(hdr.image_version),
           (unsigned)EFW_VERSION_MINOR(hdr.image_version),
           (unsigned)EFW_VERSION_PATCH(hdr.image_version),
           (unsigned)hdr.entry_addr);
    flash_unmap(flash);
    return 0;
}
