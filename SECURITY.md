# Security Policy

## Reporting a vulnerability

Report privately to **security@embeddedos.org**. Do not open a public issue for
an exploitable defect. Expect an acknowledgement within 5 working days.

## Threat model

eFirmware produces and validates the container that eBoot loads and executes.
The asset being protected is **what code runs on the device**.

### In scope

| Threat | Mitigation | State |
|---|---|---|
| Corrupted image accepted (flash wear, truncated transfer, bit rot) | SHA-256 over the payload, checked by `efw_image_verify()` | **Implemented**, per-bit tamper detection tested |
| Malformed header driving an over-long read | `efw_image_parse()` validates `hdr_size` and rejects `sig_len > 64` before any caller can size a read from them | **Implemented**, tested |
| Timing oracle during digest comparison | `efw_sha256_equal()` is constant-time | **Implemented** |
| Digest state left in memory after use | `efw_sha256_final()` zeroes the context | **Implemented** |
| Attacker-substituted image (payload **and** header rewritten) | Requires signature verification | **NOT implemented** — see below |
| Downgrade to a known-vulnerable version | Requires anti-rollback | **NOT implemented** |

### Not in scope, and not yet mitigated

**Authenticity.** `efw_image_verify()` proves a payload matches the digest in
its own header. It does not prove who produced the image. An attacker able to
write both the payload and its header produces an image that verifies cleanly.
Nothing in this repository closes that gap today:
`efw_image_attach_signature()` only stores bytes a caller computed elsewhere,
and no code here generates or checks a signature.

Consequences for integrators, stated plainly:

- Do not treat a successful `efwtool verify` as an authorisation decision.
- A device that must only run vendor firmware needs signature verification in
  the bootloader. eBoot has `EBLDR_REQUIRE_SIGNATURES`; that is where the
  check belongs until eFirmware implements one.
- Transport-level protection (TLS, a signed manifest) is currently the only
  thing standing between a hostile network and a device update.

**Confidentiality.** Payloads are not encrypted. `EFW_IMG_FLAG_ENCRYPTED`
exists for wire compatibility with eBoot; this repository neither encrypts nor
decrypts.

**Anti-rollback.** `EFW_ERR_ANTI_ROLLBACK` exists for wire compatibility. No
version-monotonicity check is performed.

## Review triggers

A change needs a security review before merge when it touches:

- `src/efw_sha256.c`, `src/efw_image.c`, or anything under `include/efw/`
- the image format, any field width, or any status code value
- parsing of externally supplied bytes
- comparison of digests, signatures, or any other secret-dependent value

## Supported versions

Pre-1.0. Only `master` receives fixes.
