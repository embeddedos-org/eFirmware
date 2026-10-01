// SPDX-License-Identifier: MIT
// Device-side verify snippet for ESP32-S3 (ESP-IDF).
//
// Reads the eFirmware image back from the app partition, parses the header
// and verifies the payload digest before eBoot boots it. The eFirmware calls
// are platform-independent; only flash_map()/flash_unmap() are ESP-IDF here.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "efw/efw_image.h"

/* --- platform glue (ESP-IDF) ------------------------------------------ */
#include "esp_partition.h"
#include "spi_flash_mmap.h"

static const uint8_t *flash_map(size_t *out_len) {
    const esp_partition_t *part =
        esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                 ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
    if (!part) return NULL;
    const void *ptr = NULL;
    if (spi_flash_mmap(part->address, part->size,
                       SPI_FLASH_MMAP_DATA, &ptr, NULL) != ESP_OK)
        return NULL;
    *out_len = part->size;
    return (const uint8_t *)ptr;
}

static void flash_unmap(const uint8_t *ptr) {
    spi_flash_munmap(ptr);
}
/* --- end platform glue ------------------------------------------------- */

// Returns 0 when the image in flash is intact and bootable, <0 otherwise.
int verify_esp32s3_image(void) {
    size_t flash_len = 0;
    const uint8_t *flash = flash_map(&flash_len);
    if (!flash) {
        printf("efw: no factory app partition\n");
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
        printf("efw: image overruns partition\n");
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
