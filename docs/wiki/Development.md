# Development

## Contribution source of truth

[CONTRIBUTING](https://github.com/embeddedos-org/eFirmware/blob/master/CONTRIBUTING.md)

Before proposing a change, also review the [README](https://github.com/embeddedos-org/eFirmware/blob/master/README.md). Keep changes scoped, add tests appropriate to the affected behavior, and follow the repository's current automation and review requirements.

## Build and dependency inputs found

`CMakeLists.txt`, `tests/CMakeLists.txt`, `tools/efwtool/CMakeLists.txt`.

## Tests found in the default-branch tree

`tests/CMakeLists.txt`, `tests/efw_test.h`, `tests/test_abi.c`, `tests/test_crc32.c`, `tests/test_image.c`, `tests/test_sha256.c`.

## Documented test commands

These commands are reproduced from the inspected root README or contributing guide:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build --parallel
```

```bash
ctest --test-dir build --output-on-failure
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEFW_SANITIZE=ON
```

## Verification baseline

This inventory comes from `master` at [`8861142b8752`](https://github.com/embeddedos-org/eFirmware/commit/8861142b875287598321f0b01e13cae13f705ac2) and found 6 test-related paths among 44 files. Re-check the source tree when that commit is no longer current.
