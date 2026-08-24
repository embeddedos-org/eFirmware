# Contributing to eFirmware

## Ground rules

1. `ctest` must stay green and the build warning-free. CI enforces both across
   Linux, macOS and Windows in Debug and Release, plus an ASan/UBSan run.
2. New behaviour arrives with tests that **can fail**. Before opening a PR,
   break your own change deliberately and confirm a test catches it. A test
   that passes against a broken implementation is worse than no test, because
   it reports safety that does not exist.
3. Anything touching the image format is an on-the-wire change. Update
   `eBoot/include/eos_image.h` in the same change set, or say explicitly why
   compatibility is intentionally broken.
4. Do not claim a property the repository cannot demonstrate. "Verified",
   "secure" and "production ready" require evidence a reader can re-run.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEFW_SANITIZE=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Style

- C11, no external dependencies in `src/` or `include/`.
- SPDX header on every file.
- Public functions get a Doxygen block with parameters, return values and a
  working example.
- Comments explain constraints the code cannot show — not what the next line
  does.

## Cryptographic code

Changes to `efw_sha256.c` must keep the NIST vectors in `tests/test_sha256.c`
passing, and comparisons of secret-dependent values stay constant-time. If you
add a primitive, add published test vectors for it in the same commit.
