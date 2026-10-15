# Board profiles

One file per board eFirmware targets: how the firmware lays out
that board's flash, boots it, partitions its security domains, and
fits the `efw_image` A/B slot scheme to its real memory geometry.

## Profiles

| Board | Chip | Status |
|---|---|---|
| [NuMaker-GestureAI-M55M1](numaker-gestureai-m55m1.md) | Nuvoton NuMicro M55M1 (Cortex-M55 + Ethos-U55) | Bring-up profile, 2026-10-11 |

## The format

Profiles follow the structure of the M55M1 profile, in this order:

1. **Header block** — date, status, and a one-paragraph statement
   of why this board matters to the org's tracks.
2. **Board and chip (sourced)** — silicon, clocks, memory sizes,
   security features, and kit peripherals, each traceable to a
   vendor source or announcement.
3. **Flash map** — the proposed eFirmware layout as a table of
   regions (offset range, size, purpose): secure loader, Slot A,
   Slot B, update state.
4. **Boot flow** — power-on to application, including the
   trial-boot and rollback path.
5. **Security partitioning** — TrustZone (or equivalent) split for
   the board.
6. **`efw_image` slot layout** — header and payload placement for
   this board's geometry.
7. **Board-specific handling** — anything the generic design does
   not cover (removable-media model slots, unusual peripherals).
8. **Links** — vendor pages, datasheets, and the org issues the
   profile serves.

## The sourcing rule

Flash geometry is never invented. Memory sizes, region boundaries
imposed by the silicon, and security capabilities come from
vendor datasheets or vendor product pages and are marked
**sourced**. Layouts eFirmware proposes on top of that geometry
are marked **proposed**. A profile whose geometry cannot yet be
sourced stays a stub naming the missing datasheet — it does not
ship guessed numbers.

Candidate boards waiting on sourced geometry are tracked in the
eCAD-Hardware-Products Incoming watchlist and the eAI silicon
roster; a profile is written when the datasheet lands, not
before.
