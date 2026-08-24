# Verification

Every item gets one result: **PASS / FAIL / NOT RUN / UNKNOWN**. A category
that does not apply is recorded NOT RUN *with a reason*, never dropped — an
absent row reads as coverage that does not exist.

## Gate

```sh
# 1. Build, warnings are errors
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 2. Tests
ctest --test-dir build --output-on-failure

# 3. Sanitizers
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DEFW_SANITIZE=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure

# 4. End-to-end
head -c 65536 /dev/urandom > payload.bin
./build/tools/efwtool/efwtool pack payload.bin fw.img --version 1.2.3
./build/tools/efwtool/efwtool verify fw.img
[ "$(sha256sum payload.bin | cut -d' ' -f1)" \
  = "$(./build/tools/efwtool/efwtool inspect fw.img | awk '/^sha256/{print $2}')" ]
```

CI runs all four across Linux, macOS and Windows in Debug and Release.

## Last recorded run

Host: Linux 6.18 x86_64, GCC 15.2.0, CMake 4.2.3.

| Item | Result | Evidence |
|---|---|---|
| Build (Release, `-Werror`) | **PASS** | 0 errors, 0 warnings |
| Unit tests | **PASS** | `100% tests passed, 0 tests failed out of 3` |
| Mutation check | **PASS** | Two seeded defects both caught (see TESTING.md) |
| Digest vs. `sha256sum` | **PASS** | Identical on a 5000-byte random payload |
| Tamper rejection | **PASS** | One flipped bit → `ERR_CRC`, exit 1 |
| Truncation rejection | **PASS** | 100-byte file → `ERR_NO_IMAGE`, exit 1 |
| Sanitizers (ASan/UBSan) | **NOT RUN** locally | Configured and exercised by the CI `sanitizers` job; not run on this host |
| Cross-compiled target build | **NOT RUN** | No cross toolchain installed here |
| Coverage | **NOT RUN** | Not instrumented yet; no figure is published |
| Performance | **NOT RUN** | No budget declared |
| UI / UX | **NOT RUN** | No user interface in this repository |

## Claims this repository does *not* make

Not "production ready", not "fully tested", not "secure". What is claimed is
exactly what the table above shows, and authenticity is explicitly unsolved —
see SECURITY.md.
