# Tasks

## Done

- [x] Image container byte-compatible with eBoot's `eos_image_header_t`
      — *acceptance:* round-trip parse of a packed image; documented byte
      offsets asserted in `tests/test_image.c`.
- [x] SHA-256 (FIPS 180-4), streaming
      — *acceptance:* 4 NIST vectors pass; digest agrees with `sha256sum`.
- [x] CRC-32/ISO-HDLC
      — *acceptance:* published check values pass; result independent of
      chunk boundaries.
- [x] Malformed-header rejection
      — *acceptance:* bad magic, unknown version, wrong `hdr_size` and
      oversized `sig_len` each return their documented status.
- [x] `efwtool` pack / inspect / verify
      — *acceptance:* CI round trip passes and rejects a tampered image.
- [x] CI on 3 OSes × 2 configs + ASan/UBSan
      — *acceptance:* workflow green.

## Open, in dependency order

- [ ] **Ed25519 signature generation and verification.** Closes the
      authenticity gap documented in SECURITY.md — the single most important
      remaining item, because `efw_image_verify()` today proves integrity only.
      *Acceptance:* a signature made with a known key verifies; a signature
      from a different key is rejected; an unsigned image is rejected when
      signatures are required. RFC 8032 test vectors must pass.
- [ ] **Anti-rollback / version policy.** *Acceptance:* an image whose
      `image_version` is below the stored floor returns
      `EFW_ERR_ANTI_ROLLBACK`.
- [ ] **Device-side update agent** — slot management, staging, commit,
      rollback. *Acceptance:* a power cut simulated at each stage leaves the
      device bootable.
- [ ] **Transports** implementing eBoot's `eos_fw_transport_ops_t`
      (UART XMODEM/YMODEM, USB DFU, network). *Acceptance:* an image
      transferred over each transport verifies byte-identical to the source.
- [ ] **Board bring-up and peripheral drivers.** Scope not yet defined; needs
      the board list settled against eCAD-Hardware-Products.
- [ ] **Coverage instrumentation.** *Acceptance:* a coverage figure is measured
      and published, replacing the current "not measured" note.

## Decisions

- **Tests default ON for host builds.** eBoot and eAI both default
  `*_BUILD_TESTS` to OFF, which is how eAI's 24 genuine C tests came to be
  never run by a default build. The only condition that disables them here is
  cross-compilation.
- **Explicit little-endian serialisation** rather than writing the struct
  directly, so the on-flash format does not silently inherit host padding or
  byte order.
- **Table-free CRC-32.** A 1 KiB lookup table is a poor trade where flash is
  scarce and verification is not on a hot path.
