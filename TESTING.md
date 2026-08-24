# Testing

## Running

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEFW_SANITIZE=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Tests are on by default for host builds. They switch off only when
cross-compiling, where there is no machine to run them on.

## Suites

| ctest name | File | Covers |
|---|---|---|
| `efw_sha256` | `tests/test_sha256.c` | 4 NIST vectors; one-byte-at-a-time equivalence; 55/56/63/64-byte padding boundaries; 10^6-byte length carry; constant-time comparison; zero-length `update()` |
| `efw_crc32` | `tests/test_crc32.c` | Published check values; chunk-boundary independence at all 10 split points; NULL/zero handling; single-bit sensitivity |
| `efw_image` | `tests/test_image.c` | Build; serialise/parse round trip; documented byte offsets; every single-bit payload flip (128 cases); length mismatch; bad magic; unknown header version; wrong header size; oversized and boundary `sig_len`; signature attach, padding and round trip; status strings |

## The rule that matters

**A test that cannot fail is not a test.** Before a change lands, break the
implementation on purpose and confirm a test catches it.

This suite has been checked that way. Corrupting one SHA-256 round constant
(`0x428a2f98` → `0x428a2f99`) and short-circuiting the digest comparison in
`efw_image_verify()` produced:

```
The following tests FAILED:
	  1 - efw_sha256 (Failed)
	  3 - efw_image (Failed)
```

Re-run that check whenever the suite grows. A suite that stays green against a
sabotaged implementation is reporting safety that does not exist.

## Independent cross-check

`efw_sha256` is validated against published NIST vectors in the unit tests and,
in CI, against the platform `sha256sum` over a 64 KiB random payload. Agreeing
with a second implementation catches whole classes of error that self-consistent
tests cannot.

## Test matrix status

| # | Category | State | Note |
|---|---|---|---|
| 1 | Unit | **PASS** | 3 suites, `ctest` green |
| 2 | Integration | **PASS** | `efwtool` exercises the library end to end in CI |
| 3 | Functional | **PASS** | pack / inspect / verify against real files |
| 4 | End-to-end | **PASS** | CI round-trip job, including tamper rejection |
| 5 | Acceptance | **PASS** | Every implemented README claim has a covering test |
| 6 | Security | **PARTIAL** | Malformed-header and constant-time paths tested; no signature verification exists to test |
| 7 | Performance | **NOT RUN** | No budget declared; image verification is not on a hot path |
| 8 | Smoke | **PASS** | `efwtool verify` on a packed image |
| 9 | Regression | **PASS** | Full suite re-run on every change |
| 10 | UI / UX | **NOT RUN** | No user interface in this repository |

Coverage is not yet measured, so no coverage figure is published.
