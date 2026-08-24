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

### Not yet implemented

Signature generation and verification, the device-side update agent,
transports, board bring-up, and anti-rollback. See the README roadmap.
