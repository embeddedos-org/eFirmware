// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project

/**
 * @file efw_test.h
 * @brief Minimal assertion harness shared by the eFirmware test binaries.
 *
 * Each test binary returns a non-zero exit status when any check fails, which
 * is what ctest reports on. Deliberately dependency-free so the same tests run
 * on a cross-compiled target as on the host.
 */

#ifndef EFW_TEST_H
#define EFW_TEST_H

#include <stdio.h>
#include <string.h>

static int efw_tests_run;
static int efw_tests_failed;

#define EFW_CHECK(cond)                                                     \
    do {                                                                    \
        efw_tests_run++;                                                    \
        if (!(cond)) {                                                      \
            efw_tests_failed++;                                             \
            printf("  [FAIL] %s:%d  %s\n", __FILE__, __LINE__, #cond);      \
        }                                                                   \
    } while (0)

#define EFW_CHECK_EQ_INT(actual, expected)                                  \
    do {                                                                    \
        long _a = (long)(actual), _e = (long)(expected);                    \
        efw_tests_run++;                                                     \
        if (_a != _e) {                                                      \
            efw_tests_failed++;                                              \
            printf("  [FAIL] %s:%d  %s == %s (got %ld, want %ld)\n",         \
                   __FILE__, __LINE__, #actual, #expected, _a, _e);          \
        }                                                                    \
    } while (0)

#define EFW_CHECK_MEM_EQ(a, b, n)                                            \
    do {                                                                     \
        efw_tests_run++;                                                     \
        if (memcmp((a), (b), (n)) != 0) {                                    \
            efw_tests_failed++;                                              \
            printf("  [FAIL] %s:%d  %s != %s over %zu bytes\n",              \
                   __FILE__, __LINE__, #a, #b, (size_t)(n));                 \
        }                                                                    \
    } while (0)

#define EFW_TEST_REPORT(suite)                                               \
    do {                                                                     \
        printf("%s: %d checks, %d failed\n",                                 \
               (suite), efw_tests_run, efw_tests_failed);                    \
        return efw_tests_failed == 0 ? 0 : 1;                                \
    } while (0)

#endif /* EFW_TEST_H */
