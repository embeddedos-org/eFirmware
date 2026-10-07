# NPU-interconnect direction (Track 1)

Date: 2026-10-07. Sibling to `docs/esp32-p4-lp-spi-errata.md`.

## Huawei Ascend 960DT: the trajectory

Huawei has accelerated the Ascend 960DT to **Q1 2027**: 2 PFLOPS FP8, 288GB
HBM, and the 960 SuperPod (15,488 NPUs) — with near-package optics and a
**UnifiedBus** challenging NVLink (announced at Huawei Connect 2026;
kimkj.com).

This is datacenter-scale hardware. It does not change eos's MCU-class BOM
this year. But it is the direction of travel for accelerator interconnects:
optics replacing copper at package scale, unified buses replacing proprietary
fabrics, and NPU clusters sized in the tens of thousands.

## Firmware KB implication

The lesson for the eFirmware/eos HAL work is negative, not positive:
**do NOT bake PCIe-era interconnect assumptions into HAL abstractions.**
The accelerator HAL (Track 1) must treat the interconnect as a capability
query — bandwidth, latency class, topology — not as "PCIe with lanes."
The day an on-device NPU talks over CXL, UCIe, or a photonics fabric, the
HAL should not need a rewrite.

## Links

- kimkj.com — Ascend 960DT / SuperPod / UnifiedBus reporting.
- eos Track 1 accelerator-HAL profiles (tokens-per-watt tiering note,
  2026-10-07).
