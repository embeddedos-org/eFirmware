# ESP32-P4 LP-SPI errata advisory (bring-up note)

> Status: advisory, not a bug report against eFirmware code — eFirmware has
> no ESP32-P4 SPI code yet (only ESP32-S3 examples). Read this before
> starting P4 SPI bring-up.

## The errata

The ESP32-P4's low-power SPI (LP-SPI) peripheral returns **stale MISO data**
when operated in **full-duplex mode with LSB-first bit order**. The received
bits lag the actual MISO line — effectively sampling the previous bit
period — so every LSB-first full-duplex transfer reads corrupted data.

- **Affected:** ESP-IDF v6.0 through v6.0.3 (the LP-SPI driver as shipped).
- **Fixed in:** ESP-IDF v6.1 (driver works around the silicon behavior).
- **Scope:** silicon errata in the LP-SPI peripheral; the main SPI
  peripherals (SPI2/SPI3) are not affected.

## How you would meet it

Any P4 design that talks to an LSB-first SPI peripheral in full duplex
through the LP-SPI block — typically a low-power sensor or radio kept alive
in sleep — and validates against IDF ≤ 6.0.3. Symptom: reads are
deterministic but wrong (bit-shifted/stale), which looks like a wiring or
timing problem and wastes a board spin of debugging.

## Workarounds (if stuck on IDF ≤ 6.0.3)

1. **Use 3-wire (SIO) mode** — single bidirectional data line instead of
   separate MOSI/MISO. This sidesteps the MISO sampling errata entirely
   and is the preferred workaround where the peripheral supports it.
2. **Use MSB-first** where the peripheral allows it — the errata is
   specific to LSB-first ordering.
3. **Use half-duplex** transactions — the errata is specific to full-duplex.
4. **Use SPI2/SPI3** instead of LP-SPI when power budget allows.
5. **Upgrade to IDF v6.1+** — the real fix.

If none of the above is possible, the remaining option is to **accept
stale/zero MISO** on affected transfers — but do it explicitly: document
which transfers are affected and what the stale data corrupts, rather
than discovering it on the bench.

## Update 2026-10-05 — v6.1 released, backport TBD

ESP-IDF **v6.1 is released** with the LP-SPI driver fix (the workaround
for the silicon behavior). The "Affected" range above (v6.0–v6.0.3)
stands; whether the fix is backported to the 6.0.x line is still TBD —
until a backported 6.0.x exists, treat all of 6.0.x as affected. v6.1
also carries the ESP32-S31 preview (see eCAD silicon catalog).

## eFirmware implications

- P4 bring-up must pin **ESP-IDF ≥ v6.1** in the build manifest before any
  LP-SPI code is written. (Note: ESP-IDF v5.2 is EOL since Aug 2026; v6 is
  the supported line for P4 regardless.)
- When `efwtool` / packing gains a P4 target, the pack/verify scripts
  should assert the IDF version and refuse to build LP-SPI firmware on
  affected versions — fail at build time, not on the bench.
- The `examples/` tree should carry an `esp32p4/` note pointing here once
  P4 examples exist.

## References

- Espressif ESP32-P4 errata documentation (LP-SPI section)
- ESP-IDF v6.1 release notes (LP-SPI driver fix)
- eFirmware `examples/esp32s3/` — current SPI-adjacent examples (S3 only)
