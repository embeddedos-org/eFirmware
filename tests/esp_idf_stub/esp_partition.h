// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file esp_partition.h
 * @brief Host stand-in for the part of ESP-IDF's esp_partition.h that
 *        examples/esp32s3/verify_esp32s3.c uses. Declarations follow the
 *        ESP-IDF v5 API so the example compiles here as it would on target.
 */

#ifndef EFW_TEST_ESP_PARTITION_H
#define EFW_TEST_ESP_PARTITION_H

#include <stdint.h>

typedef int esp_err_t;
#define ESP_OK 0

typedef enum {
    ESP_PARTITION_TYPE_APP  = 0x00,
    ESP_PARTITION_TYPE_DATA = 0x01
} esp_partition_type_t;

typedef enum {
    ESP_PARTITION_SUBTYPE_APP_FACTORY = 0x00
} esp_partition_subtype_t;

typedef struct {
    esp_partition_type_t    type;
    esp_partition_subtype_t subtype;
    uint32_t                address;
    uint32_t                size;
} esp_partition_t;

const esp_partition_t *esp_partition_find_first(esp_partition_type_t type,
                                                esp_partition_subtype_t subtype,
                                                const char *label);

#endif /* EFW_TEST_ESP_PARTITION_H */
