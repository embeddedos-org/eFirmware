# ESP-IDF v6.1 bring-up baseline

**Status:** baseline (2026-10-08). ESP-IDF v6.1 (released September 2026)
is the bring-up baseline for Espressif targets. This document records
what changed, what it unblocks, and what to watch.

Release: https://github.com/espressif/esp-idf/releases/tag/v6.1

## What v6.1 unblocks

**ESP32-P4 Wi-Fi boots.** The P4 Wi-Fi blocker that stood since July
2026 is closed (verified on v6.1-rc1: moonmodules/projectmm
67fbe644ed1b2cfdfbc394c4dce0630d5faec1a9). The queued ESP32-P4 lane
has its radio back -- P4 bring-up proceeds on v6.1, not v5.x.

**LP-SPI MISO bug fixed.** The LP-SPI MISO bug present in v6.0-6.0.3 is
fixed in v6.1. (See also `docs/esp32-p4-lp-spi-errata.md` for the
P4-specific errata history.)

## Chip-revision targeting: name the rev

v6.1 changes the **default P4 silicon revision to v3.0**. Firmware
images must target the revision they ship for -- rev v1 vs rev v3.0
are different bring-up targets. Every P4 firmware doc in this repo
must name the silicon revision it was validated against; "ESP32-P4"
without a rev is no longer a complete target description.

## Watch item: ECDSA Secure Boot V2 disabled on H2/C5/P4

v6.1 **disables ECDSA-based Secure Boot V2 on ESP32-H2, ESP32-C5, and
ESP32-P4** "due to a security vulnerability found in the ECDSA based
Secure Boot flow," with details promised in the chip errata. It also
removes the 192-bit curve.

For eFirmware bring-up this means:

1. Do not provision new H2/C5/P4 devices against the ECDSA-SBv2 flow
   on v6.1 -- it is disabled for a reason, and the reason is a
   shipping vulnerability.
2. Track the errata: when Espressif publishes the details, assess
   whether any already-provisioned device needs re-provisioning.
3. Cross-ref the eBoot threat model, which carries the same watch
   item for the bootloader's ESP32 trust path.

## Baseline rule

New Espressif bring-up starts at **ESP-IDF v6.1**. v5.2 went end-of-life
in August 2026; v6.0.x carries the LP-SPI MISO bug. Pin v6.1 in
bring-up docs and CI.

## v6.1-rc1 notes (2026-10)

From the v6.1 release candidate notes and early bring-up:

- **ESP32-S31 preview support.** The S31 is a preview target in v6.1
  (`CONFIG_IDF_TARGET_ESP32S31` with preview targets enabled); esp-rs
  already has a basic HAL. eos tracks it as
  `boards/generic-esp32s31.yaml` (preview, conservative specs).
- **OWE-Only SoftAP** — opportunistic-wireless-encryption-only AP mode.
- **DPP multi-config** and **Wi-Fi Aware pairing for iOS**.
- **Runtime sleep-retention attach/detach.**
- **`CONFIG_SPIRAM_ENC_EXEMPT`** — unencrypted PSRAM carve-out; note
  for the threat model: exempt regions must never hold keys or
  PII-adjacent buffers.
- **JPEG decoder bad-picture fix (GHSA-v6r2-f6p2-88cj).** Logged as a
  CVE-watch datapoint for the monthly device-CVE series (embeddedos-stack
  `docs/kev-gate.md`): a decoder bug class worth watching across
  camera-adjacent firmware.
