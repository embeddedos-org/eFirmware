# eFirmware

[![CI](https://github.com/embeddedos-org/eFirmware/actions/workflows/ci.yml/badge.svg)](https://github.com/embeddedos-org/eFirmware/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C11](https://img.shields.io/badge/C-11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))

Firmware image toolkit for the EoS stack: build, sign-slot, inspect and verify
the firmware containers that [eBoot](https://github.com/embeddedos-org/eBoot)
loads.

> **Status: early.** One vertical slice is implemented and tested — the image
> container plus its SHA-256 and CRC-32 primitives, and the `efwtool` host
> utility. The device-side update agent, transports and board bring-up layers
> described in [Roadmap](#roadmap) are **not written yet**. The CI badge above
> reflects real workflow status; there is no coverage badge because coverage is
> not yet measured.

## What is implemented

| Component | State | Evidence |
|---|---|---|
| `efw_sha256` — streaming SHA-256 (FIPS 180-4) | Working | 4 NIST vectors + streaming-equivalence and padding-boundary tests |
| `efw_crc32` — CRC-32/ISO-HDLC | Working | Published check values + chunk-boundary independence |
| `efw_image` — 156-byte container, byte-compatible with eBoot | Working | Round-trip, per-bit tamper detection, malformed-header rejection |
| `efwtool` — `pack` / `inspect` / `verify` | Working | End-to-end round trip in CI, cross-checked against `sha256sum` |
| Update agent, transports, board bring-up | **Not started** | — |

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Requires CMake 3.15+ and a C11 compiler. No external dependencies.

Tests are **on by default** for host builds and switch off only when
cross-compiling, where there is no machine to run them on. Warnings are errors
by default (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`).

| Option | Default | Effect |
|---|---|---|
| `EFW_BUILD_TESTS` | `ON` (host) | Build and register the ctest suite |
| `EFW_BUILD_TOOLS` | `ON` | Build the `efwtool` utility |
| `EFW_HARDENING` | `ON` | `-fstack-protector-strong`, `_FORTIFY_SOURCE=2` |
| `EFW_WERROR` | `ON` | Treat warnings as errors |
| `EFW_SANITIZE` | `OFF` | ASan + UBSan on host builds |

## Using efwtool

```sh
# Pack a payload into an image
./build/tools/efwtool/efwtool pack firmware.bin firmware.img \
    --load 0x08010000 --entry 0x08010100 --version 1.2.3

# Show the header
./build/tools/efwtool/efwtool inspect firmware.img

# Verify payload integrity against the header digest
./build/tools/efwtool/efwtool verify firmware.img && echo intact
```

`verify` exits non-zero on a digest mismatch, a length mismatch, or a
malformed header, so it can gate a release step directly.

## Image format

An image file is a 156-byte little-endian header followed by the payload. The
layout is byte-compatible with `eos_image_header_t` in
`eBoot/include/eos_image.h`, so eBoot's `eos_image_parse_header()` consumes it
without translation.

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 4 | `magic` | `0x454F5349` ("EOSI") |
| 4 | 2 | `hdr_version` | `1` |
| 6 | 2 | `hdr_size` | `156` |
| 8 | 4 | `image_size` | Payload bytes, excluding the header |
| 12 | 4 | `load_addr` | |
| 16 | 4 | `entry_addr` | |
| 20 | 4 | `image_version` | `0xMMmmpppp` |
| 24 | 4 | `flags` | `EFW_IMG_FLAG_*` |
| 28 | 32 | `hash` | SHA-256 of the payload |
| 60 | 1 | `sig_type` | `efw_sig_type_t` |
| 61 | 1 | `sig_len` | Bytes of `signature` in use |
| 62 | 30 | `reserved` | Must be zero |
| 92 | 64 | `signature` | |

Changing any constant above is an on-the-wire format change and must be
matched in eBoot.

## What this does not do

Being explicit, because integrity and authenticity are easy to conflate:

- **`efw_image_verify()` checks integrity, not authenticity.** It confirms the
  payload matches the digest in its own header. An attacker who rewrites both
  the payload and the header passes this check.
- **No signing is implemented.** `efw_image_attach_signature()` stores a
  signature a caller computed elsewhere; nothing here generates or validates
  one. Callers that must reject unsigned images have to test
  `EFW_IMG_FLAG_SIGNED` and verify the signature with their own key material.
- **No anti-rollback.** `EFW_ERR_ANTI_ROLLBACK` exists for wire compatibility
  with eBoot; no version-monotonicity check is performed here.

## Roadmap

Not yet started, in dependency order:

1. Signature generation and verification (Ed25519), closing the authenticity gap.
2. Device-side update agent — slot management, staging, commit and rollback.
3. Transports implementing eBoot's `eos_fw_transport_ops_t` (UART XMODEM/YMODEM, USB DFU, network).
4. Board bring-up and peripheral drivers.
5. Anti-rollback and version policy.

## Layout

```
include/efw/    public headers
src/            library implementation
tools/efwtool/  host utility
tests/          ctest suite, dependency-free harness
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Every change must keep `ctest` green
and the build warning-free; CI enforces both on Linux, macOS and Windows in
Debug and Release, plus an ASan/UBSan run.

## License

MIT — see [LICENSE](LICENSE).
