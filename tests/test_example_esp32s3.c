// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// examples/esp32s3/verify_esp32s3.c, built on the host against stand-ins for
// the ESP-IDF partition and SPI-flash mmap calls (tests/esp_idf_stub/).

#include "efw/efw_image.h"
#include "efw_test.h"

#include "esp_partition.h"
#include "spi_flash_mmap.h"

#include <stdlib.h>

int verify_esp32s3_image(void);

#include "../examples/esp32s3/verify_esp32s3.c"

/* ---- simulated factory partition ----------------------------------------- */

#define PART_ADDR 0x10000u
#define PART_SIZE 4096u
#define MAP_HANDLE 0x5EEDu

static uint8_t flash_part[PART_SIZE];
static const esp_partition_t factory = {
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY,
    PART_ADDR, PART_SIZE
};
static int maps_live;
static int bad_unmaps;

const esp_partition_t *esp_partition_find_first(esp_partition_type_t type,
                                                esp_partition_subtype_t subtype,
                                                const char *label)
{
    (void)label;
    if (type != ESP_PARTITION_TYPE_APP ||
        subtype != ESP_PARTITION_SUBTYPE_APP_FACTORY) return NULL;
    return &factory;
}

esp_err_t spi_flash_mmap(size_t src_addr, size_t size,
                         spi_flash_mmap_memory_t memory,
                         const void **out_ptr,
                         spi_flash_mmap_handle_t *out_handle)
{
    (void)memory;
    if (src_addr != PART_ADDR || size != PART_SIZE) return -1;
    /* ESP-IDF stores the handle unconditionally, so NULL is not allowed. */
    if (!out_ptr || !out_handle) {
        printf("  spi_flash_mmap: out_handle is NULL (ESP-IDF writes through it)\n");
        abort();
    }
    *out_ptr = flash_part;
    *out_handle = MAP_HANDLE;
    maps_live++;
    return ESP_OK;
}

void spi_flash_munmap(spi_flash_mmap_handle_t handle)
{
    if (handle != MAP_HANDLE) {
        bad_unmaps++;
        return;
    }
    maps_live--;
}

static void write_image(const uint8_t *payload, size_t len)
{
    efw_image_params_t p;
    efw_image_header_t h;

    memset(&p, 0, sizeof(p));
    p.load_addr = PART_ADDR;
    p.entry_addr = PART_ADDR;
    p.image_version = EFW_VERSION_MAKE(1, 0, 0);

    memset(flash_part, 0xFF, sizeof(flash_part));
    EFW_CHECK_EQ_INT(efw_image_build(&p, payload, len, &h), EFW_OK);
    EFW_CHECK_EQ_INT(efw_image_serialize(&h, flash_part), EFW_OK);
    memcpy(flash_part + EFW_IMAGE_HDR_SIZE, payload, len);
}

int main(void)
{
    static const uint8_t payload[] = "eFirmware ESP32-S3 example payload";

    /* An intact image verifies, and the mapping is released with the handle
     * spi_flash_mmap() returned. */
    write_image(payload, sizeof(payload));
    EFW_CHECK_EQ_INT(verify_esp32s3_image(), 0);
    EFW_CHECK_EQ_INT(maps_live, 0);
    EFW_CHECK_EQ_INT(bad_unmaps, 0);

    /* A tampered payload is refused, and the mapping is still released. */
    flash_part[EFW_IMAGE_HDR_SIZE] = (uint8_t)(flash_part[EFW_IMAGE_HDR_SIZE] ^ 0x01u);
    EFW_CHECK_EQ_INT(verify_esp32s3_image(), -1);
    EFW_CHECK_EQ_INT(maps_live, 0);
    EFW_CHECK_EQ_INT(bad_unmaps, 0);

    /* No header at all (erased flash) is refused the same way. */
    memset(flash_part, 0xFF, sizeof(flash_part));
    EFW_CHECK_EQ_INT(verify_esp32s3_image(), -1);
    EFW_CHECK_EQ_INT(maps_live, 0);
    EFW_CHECK_EQ_INT(bad_unmaps, 0);

    /* An image_size larger than the partition is refused before the payload is
     * hashed. UINT32_MAX is the case the old `EFW_IMAGE_HDR_SIZE + payload_len`
     * form let through on a 32-bit target, where that sum wraps to 155. */
    write_image(payload, sizeof(payload));
    flash_part[8]  = 0xFFu;
    flash_part[9]  = 0xFFu;
    flash_part[10] = 0xFFu;
    flash_part[11] = 0xFFu;
    EFW_CHECK_EQ_INT(verify_esp32s3_image(), -1);
    EFW_CHECK_EQ_INT(maps_live, 0);
    EFW_CHECK_EQ_INT(bad_unmaps, 0);

    EFW_TEST_REPORT("example_esp32s3");
}
