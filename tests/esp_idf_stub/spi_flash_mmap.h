// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file spi_flash_mmap.h
 * @brief Host stand-in for ESP-IDF's spi_flash_mmap.h. The prototypes match
 *        ESP-IDF v5: spi_flash_mmap() writes the mapping handle through
 *        out_handle (it does not accept NULL), and spi_flash_munmap() takes
 *        that handle, not the mapped pointer.
 */

#ifndef EFW_TEST_SPI_FLASH_MMAP_H
#define EFW_TEST_SPI_FLASH_MMAP_H

#include <stddef.h>
#include <stdint.h>

#include "esp_partition.h"

typedef enum {
    SPI_FLASH_MMAP_DATA,
    SPI_FLASH_MMAP_INST
} spi_flash_mmap_memory_t;

typedef uint32_t spi_flash_mmap_handle_t;

esp_err_t spi_flash_mmap(size_t src_addr, size_t size,
                         spi_flash_mmap_memory_t memory,
                         const void **out_ptr,
                         spi_flash_mmap_handle_t *out_handle);

void spi_flash_munmap(spi_flash_mmap_handle_t handle);

#endif /* EFW_TEST_SPI_FLASH_MMAP_H */
