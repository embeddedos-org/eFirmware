// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file main.c
 * @brief efwtool — pack, inspect and verify eFirmware images.
 *
 * Usage:
 *   efwtool pack    <payload> <output> [--load ADDR] [--entry ADDR]
 *                                     [--version M.m.p] [--flags N]
 *   efwtool inspect <image>
 *   efwtool verify  <image>
 *
 * An image file is the 156-byte header followed by the payload.
 */

#include "efw/efw_image.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EFW_TOOL_MAX_IMAGE (64u * 1024u * 1024u)

static int read_file(const char *path, uint8_t **buf, size_t *len)
{
    FILE *f = fopen(path, "rb");
    long size;

    *buf = NULL;
    *len = 0;
    if (!f) {
        fprintf(stderr, "efwtool: cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    size = ftell(f);
    if (size < 0) { fclose(f); return -1; }
    if ((unsigned long)size > EFW_TOOL_MAX_IMAGE) {
        fprintf(stderr, "efwtool: %s is larger than the %u byte limit\n",
                path, EFW_TOOL_MAX_IMAGE);
        fclose(f);
        return -1;
    }
    rewind(f);

    *buf = (uint8_t *)malloc((size_t)size ? (size_t)size : 1u);
    if (!*buf) { fclose(f); return -1; }
    if (size > 0 && fread(*buf, 1u, (size_t)size, f) != (size_t)size) {
        fprintf(stderr, "efwtool: short read on %s\n", path);
        free(*buf);
        *buf = NULL;
        fclose(f);
        return -1;
    }
    *len = (size_t)size;
    fclose(f);
    return 0;
}

static int parse_version(const char *s, uint32_t *out)
{
    unsigned major = 0, minor = 0, patch = 0;
    if (sscanf(s, "%u.%u.%u", &major, &minor, &patch) != 3) return -1;
    if (major > 0xFFu || minor > 0xFFu || patch > 0xFFFFu) return -1;
    *out = EFW_VERSION_MAKE(major, minor, patch);
    return 0;
}

static void print_hex(const uint8_t *d, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) printf("%02x", d[i]);
}

static int cmd_pack(int argc, char **argv)
{
    efw_image_params_t p;
    efw_image_header_t h;
    uint8_t wire[EFW_IMAGE_HDR_SIZE];
    uint8_t *payload = NULL;
    size_t payload_len = 0;
    const char *in, *out;
    FILE *f;
    int i, rc;

    if (argc < 2) {
        fprintf(stderr, "efwtool pack: need <payload> <output>\n");
        return 2;
    }
    in = argv[0];
    out = argv[1];

    memset(&p, 0, sizeof(p));
    p.image_version = EFW_VERSION_MAKE(0, 1, 0);

    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--load") == 0 && i + 1 < argc) {
            p.load_addr = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "--entry") == 0 && i + 1 < argc) {
            p.entry_addr = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "--flags") == 0 && i + 1 < argc) {
            p.flags = (uint32_t)strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "--version") == 0 && i + 1 < argc) {
            if (parse_version(argv[++i], &p.image_version) != 0) {
                fprintf(stderr, "efwtool: bad --version, expected M.m.p\n");
                return 2;
            }
        } else {
            fprintf(stderr, "efwtool: unknown option %s\n", argv[i]);
            return 2;
        }
    }

    if (read_file(in, &payload, &payload_len) != 0) return 1;

    rc = efw_image_build(&p, payload, payload_len, &h);
    if (rc != EFW_OK) {
        fprintf(stderr, "efwtool: build failed: %s\n", efw_status_str(rc));
        free(payload);
        return 1;
    }
    if (efw_image_serialize(&h, wire) != EFW_OK) {
        free(payload);
        return 1;
    }

    f = fopen(out, "wb");
    if (!f) {
        fprintf(stderr, "efwtool: cannot write %s: %s\n", out, strerror(errno));
        free(payload);
        return 1;
    }
    if (fwrite(wire, 1u, sizeof(wire), f) != sizeof(wire) ||
        (payload_len > 0 &&
         fwrite(payload, 1u, payload_len, f) != payload_len)) {
        fprintf(stderr, "efwtool: short write on %s\n", out);
        fclose(f);
        free(payload);
        return 1;
    }
    fclose(f);
    free(payload);

    printf("packed %s -> %s (%zu byte payload, header %u bytes)\n",
           in, out, payload_len, EFW_IMAGE_HDR_SIZE);
    return 0;
}

static int load_image(const char *path, uint8_t **raw, size_t *raw_len,
                      efw_image_header_t *h)
{
    int rc;

    if (read_file(path, raw, raw_len) != 0) return 1;
    rc = efw_image_parse(*raw, *raw_len, h);
    if (rc != EFW_OK) {
        fprintf(stderr, "efwtool: %s is not a valid image: %s\n",
                path, efw_status_str(rc));
        free(*raw);
        *raw = NULL;
        return 1;
    }
    return 0;
}

static int cmd_inspect(int argc, char **argv)
{
    efw_image_header_t h;
    uint8_t *raw = NULL;
    size_t raw_len = 0;

    if (argc < 1) {
        fprintf(stderr, "efwtool inspect: need <image>\n");
        return 2;
    }
    if (load_image(argv[0], &raw, &raw_len, &h) != 0) return 1;

    printf("magic         0x%08X\n", h.magic);
    printf("hdr_version   %u\n", h.hdr_version);
    printf("hdr_size      %u\n", h.hdr_size);
    printf("image_size    %u\n", h.image_size);
    printf("load_addr     0x%08X\n", h.load_addr);
    printf("entry_addr    0x%08X\n", h.entry_addr);
    printf("image_version %u.%u.%u\n",
           EFW_VERSION_MAJOR(h.image_version),
           EFW_VERSION_MINOR(h.image_version),
           EFW_VERSION_PATCH(h.image_version));
    printf("flags         0x%08X\n", h.flags);
    printf("sha256        ");
    print_hex(h.hash, EFW_SHA256_DIGEST_LEN);
    printf("\n");
    printf("sig_type      %u\n", h.sig_type);
    printf("sig_len       %u\n", h.sig_len);
    printf("file_size     %zu (payload on disk %zu)\n",
           raw_len, raw_len - EFW_IMAGE_HDR_SIZE);

    free(raw);
    return 0;
}

static int cmd_verify(int argc, char **argv)
{
    efw_image_header_t h;
    uint8_t *raw = NULL;
    size_t raw_len = 0;
    int rc;

    if (argc < 1) {
        fprintf(stderr, "efwtool verify: need <image>\n");
        return 2;
    }
    if (load_image(argv[0], &raw, &raw_len, &h) != 0) return 1;

    rc = efw_image_verify(&h, raw + EFW_IMAGE_HDR_SIZE,
                          raw_len - EFW_IMAGE_HDR_SIZE);
    if (rc == EFW_OK) {
        printf("%s: OK (%u bytes, sha256 ", argv[0], h.image_size);
        print_hex(h.hash, EFW_SHA256_DIGEST_LEN);
        printf(")\n");
    } else {
        fprintf(stderr, "%s: FAILED (%s)\n", argv[0], efw_status_str(rc));
    }
    free(raw);
    return rc == EFW_OK ? 0 : 1;
}

static void usage(void)
{
    fprintf(stderr,
        "efwtool - eFirmware image utility\n\n"
        "  efwtool pack <payload> <output> [--load ADDR] [--entry ADDR]\n"
        "                                 [--version M.m.p] [--flags N]\n"
        "  efwtool inspect <image>\n"
        "  efwtool verify  <image>\n");
}

int main(int argc, char **argv)
{
    if (argc < 2) { usage(); return 2; }
    if (strcmp(argv[1], "pack") == 0)    return cmd_pack(argc - 2, argv + 2);
    if (strcmp(argv[1], "inspect") == 0) return cmd_inspect(argc - 2, argv + 2);
    if (strcmp(argv[1], "verify") == 0)  return cmd_verify(argc - 2, argv + 2);
    usage();
    return 2;
}
