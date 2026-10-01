# STM32F4 example

End-to-end example: pack an application binary for the STM32F4 with
`efwtool`, flash it with `st-flash` (or OpenOCD), and verify it on-device
with the eFirmware C API before handing it to eBoot.

This is the second reference target (after ESP32-S3), proving eFirmware's
image format and tooling are portable across vendors and architectures.

## Layout

| File | Purpose |
|---|---|
| `pack-stm32f4.sh` | Packs `<payload.bin>` into an eFirmware image with STM32F4 flash defaults |
| `verify_stm32f4.c` | Device-side snippet: parse + verify the image from flash |

## Pack

```sh
# Build efwtool first (host)
cmake -S /path/to/eFirmware -B /path/to/eFirmware/build
cmake --build /path/to/eFirmware/build --target efwtool

# Pack your application binary (e.g. a bare-metal ELF's .bin)
./pack-stm32f4.sh build/app.bin build/app-efw.img --version 1.0.0

# Inspect / verify on the host
/path/to/eFirmware/build/tools/efwtool/efwtool inspect build/app-efw.img
/path/to/eFirmware/build/tools/efwtool/efwtool verify  build/app-efw.img
```

Defaults used by the script (override with `--load` / `--entry`):

- `--load 0x08010000` — application slot in STM32F4 flash, leaving the
  first 64 KiB (0x08000000–0x0800FFFF) for eBoot. Matches the load address
  eFirmware's own CI round-trip exercises.
- `--entry 0x08010100` — entry point just past the 156-byte eFirmware
  header plus alignment, so a raw payload executes in place.

## Flash

```sh
# With st-flash (STM32CubeProgrammer CLI also works)
st-flash write build/app-efw.img 0x08010000

# Or with OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/app-efw.img 0x08010000 verify reset exit"
```

## Verify on device

`verify_stm32f4.c` shows the device-side check: the STM32F4 maps flash
directly into the address space at 0x08000000, so `flash_map()` is just a
pointer to the image's load address — no HAL flash API needed.
`efw_image_parse()` the 156-byte header (it rejects malformed headers
without over-reading), then `efw_image_verify()` the payload against the
header SHA-256. Only hand a verified image to eBoot's
`eos_image_parse_header()` / boot path.

The snippet uses no vendor headers at all; the eFirmware calls are
platform-independent.

## Notes

- eFirmware verifies **integrity**, not authenticity. For production,
  attach a signature with `efw_image_attach_signature()` and check
  `EFW_IMG_FLAG_SIGNED` plus the signature with your own key material
  before booting.
- The header is little-endian and byte-compatible with eBoot's
  `eos_image_header_t`, so eBoot consumes the image without translation.
- Reference: [ESP32-S3 example](../esp32s3/README.md).
