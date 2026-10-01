# ESP32-S3 example

End-to-end example: pack an application binary for the ESP32-S3 with
`efwtool`, flash it with `esptool.py`, and verify it on-device with the
eFirmware C API before handing it to eBoot.

## Layout

| File | Purpose |
|---|---|
| `pack-esp32s3.sh` | Packs `<payload.bin>` into an eFirmware image with ESP32-S3 flash defaults |
| `verify_esp32s3.c` | Device-side snippet: parse + verify the image from flash |

## Pack

```sh
# Build efwtool first (host)
cmake -S /path/to/eFirmware -B /path/to/eFirmware/build
cmake --build /path/to/eFirmware/build --target efwtool

# Pack your application binary (e.g. an ESP-IDF app .bin)
./pack-esp32s3.sh build/app.bin build/app-efw.img --version 1.0.0

# Inspect / verify on the host
/path/to/eFirmware/build/tools/efwtool/efwtool inspect build/app-efw.img
/path/to/eFirmware/build/tools/efwtool/efwtool verify  build/app-efw.img
```

Defaults used by the script (override with `--load` / `--entry`):

- `--load 0x10000` — the ESP32-S3 factory application partition offset in
  the default ESP-IDF partition table (the ROM bootloader + second-stage
  bootloader occupy the first 64 KiB of flash).
- `--entry 0x10000` — raw payloads execute in place from their load address.

## Flash

```sh
# Erase, then write the image at the app partition offset
esptool.py --chip esp32s3 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x10000 build/app-efw.img
```

## Verify on device

`verify_esp32s3.c` shows the device-side check: map the flash region,
`efw_image_parse()` the 156-byte header (it rejects malformed headers
without over-reading), then `efw_image_verify()` the payload against the
header SHA-256. Only hand a verified image to eBoot's
`eos_image_parse_header()` / boot path.

The snippet is written against the ESP-IDF SPI-flash mmap API; adapt the
`flash_map()`/`flash_unmap()` helpers to your platform (bare-metal,
Arduino, Zephyr) — the eFirmware calls are platform-independent.

## Notes

- eFirmware verifies **integrity**, not authenticity. For production,
  attach a signature with `efw_image_attach_signature()` and check
  `EFW_IMG_FLAG_SIGNED` plus the signature with your own key material
  before booting.
- The header is little-endian and byte-compatible with eBoot's
  `eos_image_header_t`, so eBoot consumes the image without translation.
