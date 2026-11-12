# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `efw_sha256` — streaming SHA-256 (FIPS 180-4) with constant-time digest
  comparison. Verified against four NIST vectors, a streaming-equivalence
  check, and the 55/56/63/64-byte padding boundaries.
- `efw_crc32` — CRC-32/ISO-HDLC, table-free. Verified against published check
  values and for chunk-boundary independence.
- `efw_image` — 156-byte firmware container, byte-compatible with eBoot's
  `eos_image_header_t`. Build, serialise, parse and verify, with rejection of
  malformed headers (bad magic, unknown header version, wrong header size,
  oversized `sig_len`) and per-bit payload tamper detection.
- `efwtool` — host utility with `pack`, `inspect` and `verify` subcommands.
- CI across Linux, macOS and Windows in Debug and Release, an ASan/UBSan job,
  and an end-to-end round trip that cross-checks the recorded digest against
  `sha256sum` and confirms a tampered image is rejected.

### Fixed

- `efwtool pack` now refuses a malformed `--load`, `--entry`, `--flags` or
  `--version` value and exits 2 without writing an image. It used to keep the
  parsed prefix or truncate the value, and wrote an image with a wrong header
  field. `--load`, `--entry` and `--flags` must now be a single decimal, `0x`
  hex or leading-`0` octal number that fits in 32 bits; values that were
  accepted before and are now refused include trailing text (`0x0801000G`),
  more than 32 bits, a sign (`-1`, `+5`), spaces before or after the number,
  a bare `0x` (was 0) and `08`/`09` (were 0). `--version` must be exactly
  `M.m.p` (`1.2.3x` and `1.2.3.4` were read as 1.2.3).
- `efwtool inspect` and `efwtool verify` now open an image made from the
  largest payload `efwtool pack` accepts (64 MiB). They applied the 64 MiB
  limit to the whole file, header included, so an image whose payload was
  within 155 bytes of the limit was packed but then refused as too large.

### Not yet implemented

Signature generation and verification, the device-side update agent,
transports, board bring-up, and anti-rollback. See the README roadmap.
