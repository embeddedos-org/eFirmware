# NuMaker-GestureAI-M55M1 firmware profile

**Date:** 2026-10-11. Status: bring-up profile (proposed eFirmware slot layout;
numbers marked "sourced" come from the vendor/press below; anything else is a
proposal and flagged as such).

The NuMaker-GestureAI-M55M1 is the cheapest credible Ethos-U55 vision+voice
board in the Track 1 silicon watch — and eAI's silicon-roster Ethos-U55
reference NPU. This profile defines how eFirmware targets it: flash map, boot
flow, TrustZone partitioning, `efw_image` A/B slot layout for the 2 MB flash,
and microSD model-slot handling.

## Board and chip (sourced)

- MCU: **Nuvoton NuMicro M55M1R2LJAE** — 220 MHz Arm Cortex-M55 with Helium
  MVE, TrustZone, and an **Arm Ethos-U55 microNPU @ 220 MHz (111 GOPS)**.
- Memory: **1.5 MB SRAM**, **2 MB on-chip flash**. (Chip series is "up to"
  these sizes; the R2LJAE part on this module carries the full 2 MB / 1.5 MB.)
- Security: Arm TrustZone, secure boot, AES, **PSA Certified Level 2**.
- Kit: GC0308 VGA CMOS camera, on-board digital microphone (PDM/DMIC),
  microSD slot (on the back, for storing AI model files), HS-USB Type-C,
  ICE debug header, UART5 header (inference data out), I2C1 header,
  reset + user buttons, user LED, 5 V via USB-C.
- Ships with Nuvoton pre-loaded gesture/person-recognition firmware; outputs
  detection packets over UART5 as `[AA][ID][x y w h][55][CC]`.

Sources: Nuvoton NuMaker-GestureAI-M55M1 product page, Nuvoton M55M1 series
page, CNX Software (June 2026), EEWorld, Hackster.

## Flash map (2 MB, proposed eFirmware layout)

All values are byte offsets into the 2 MB internal flash unless noted.
The M55M1 maps its internal APROM at the vendor boot base; the layout below
is written in flash offsets so it survives vendor aliasing. The slot sizes
are a proposal sized for realistic Track 1 vision+voice payloads — each slot
fits a full camera pipeline + TFLM/Ethos-U55 delegate firmware.

| Region | Offset range | Size | Purpose |
|---|---|---|---|
| eBoot secure loader | `0x00000000`–`0x0001FFFF` | 128 KiB | eBoot stage 0/1 + TrustZone secure services |
| Slot A (`efw_image`) | `0x00020000`–`0x000EFFFF` | 832 KiB | Primary application image |
| Slot B (`efw_image`) | `0x000F0000`–`0x001BFFFF` | 832 KiB | Update target / rollback image |
| Update state | `0x001C0000`–`0x001FFFFF` | 256 KiB | Slot-active flag, trial-boot counter, model manifest |

256 + 832 + 832 + 128 = 2048 KiB = 2 MiB. ✓

Notes:

- The 128 KiB secure-loader budget and 256 KiB update-state budget are
  eFirmware proposals, not vendor facts — keep both if a real bring-up needs
  different geometry, and update this doc with the measured values.
- **Unverified:** exact APROM alias base and sector/erase granularity for the
  M55M1 (NuMicro parts typically erase in 4 KiB pages; confirm against the
  M55M1 datasheet before flashing in production). The slot boundaries above
  are 64 KiB-aligned so they remain safe under any erase block ≥ 4 KiB.
- The Nuvoton-shipped gesture firmware should be preserved in Slot A during
  development until the eFirmware build reproduces the camera→Ethos-U55
  pipeline end-to-end (the UART5 packet format above is the regression oracle).

## Boot flow

1. M55M1 reset → secure ROM (vendor) validates the secure loader in the
   128 KiB region (secure boot is a PSA L2 requirement on this part —
   provision it before anything else; **unverified**: Nuvoton's exact
   key-provisioning flow for the M55M1 — track the vendor BSP).
2. eBoot secure stage configures TrustZone SAU/IDAU (see below), parses the
   slot-active flag from update state, then `efw_image_parse()` +
   `efw_image_verify()` the active `efw_image` slot (156-byte header,
   SHA-256 payload digest; byte-compatible with eBoot's
   `eos_image_parse_header()` — see `include/efw/efw_image.h`).
3. Verified slot → jump to non-secure entry. Failed slot → try the other
   slot (rollback). Both fail → drop to a minimal USB/MSD recovery that
   exposes the microSD slot and re-flashes Slot B from an `efw_image` file.
4. Trial-boot logic lives in update state: after a successful update the
   loader marks the trial counter; the new image must confirm boot before the
   slot-active flag is committed — otherwise the loader falls back to the
   known-good slot.

## TrustZone secure / non-secure partition (PSA L2)

- **Secure region (flash):** the 128 KiB loader region plus the first 16 KiB
  of update state (slot-active flag + trial counter — rollback state must
  not be writable by non-secure code).
- **Non-secure region (flash):** Slot A, Slot B, and the remainder of update
  state (model manifest — models are non-secure data; their *integrity* is
  enforced by the manifest digests, not by TrustZone).
- **SRAM:** partition at init time. Proposed: lower 512 KiB secure
  (secure services, crypto keys, Ethos-U55 command buffers), upper 1 MiB
  non-secure (application heap, camera frame buffers, TFLM arenas). This
  split is a proposal — SAU region programming must be validated on-chip;
  keep the secure window tight per PSA L2 least-privilege expectations.
- Model verification (below) runs in secure context before the model is
  marked usable; inference itself runs non-secure.

## `efw_image` slot layout for this geometry

Each of Slot A and Slot B carries one self-describing `efw_image`:

- Pack with `efwtool` (`tools/efwtool`), `--load` = slot base offset
  (`0x00020000` / `0x000F0000`), entry just past the 156-byte header plus
  alignment — the same defaults pattern as
  `examples/stm32f4/README.md` (`--load 0x08010000`, entry header + aligned).
- Flags: set `EFW_IMG_FLAG_SIGNED` and `EFW_IMG_FLAG_HASH_SHA256` for
  production images. eFirmware checks **integrity** (SHA-256); authenticity
  needs `efw_image_attach_signature()` plus the caller's own key validation
  before the slot is marked bootable — the 256 KiB update-state region is
  where the key-provisioned policy lives.
- Payload budget per slot: ≤ 832 KiB minus header. A vision pipeline
  (GC0308 VGA driver + Ethos-U55 delegate + TFLM) fits comfortably; a
  VGA-frame buffer pool does NOT live in flash — it lives in SRAM (see
  TrustZone split above).

## microSD model-slot handling

- Models ship as files on the microSD card (FAT), not in flash — the 2 MB
  internal flash is reserved for firmware; the card is the model slot.
- Naming: `<model-name>-<target>.efwmodel`, one file per (name, quant target)
  — aligns with the ebuild model registry (`model@sha256:…` pin strings).
- On boot the secure loader reads a small **model manifest** from the 256 KiB
  update-state region: entries of `{path on SD, sha256 digest, ethos-u55
  weight size, minimum SRAM requirement}`. The loader hashes each model file
  on the card against the manifest before marking it usable; tampered models
  are refused (fail-closed).
- Verified models are copied into SRAM (or external HyperRAM when present —
  **unverified** whether this module carries the 8 MB HyperRAM seen on the
  NuMaker-M55M1 dev board; size the Ethos-U55 weight buffer against SRAM
  until confirmed) for Ethos-U55 inference.
- The microSD slot doubles as the USB/MSD recovery channel (see boot flow
  step 3): the recovery image drops an `efw_image` file on the card, and the
  loader flashes it into Slot B.

## Links

- Nuvoton NuMaker-GestureAI-M55M1 product page — nuvoton.com/board/numaker-gestureai-m55m1/
- Nuvoton M55M1 series page — direct.nuvoton.com/en/numaker-gestureai-m55m1
- CNX Software: NuMaker-GestureAI-M55M1 module specs — cnx-software.com (June 2026)
- eAI Track 1 silicon roster (M55M1 as Ethos-U55 reference NPU) — `eAI/docs/track1/silicon-roster.md`
- Machine-readable geometry — `boards/numaker-gestureai-m55m1.yaml`
