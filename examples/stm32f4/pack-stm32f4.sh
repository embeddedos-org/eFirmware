#!/bin/sh
# SPDX-License-Identifier: MIT
# Pack a payload binary into an eFirmware image with STM32F4 defaults.
#
# Usage: pack-stm32f4.sh <payload.bin> <output.img> [--version M.m.p]
#                                          [--load ADDR] [--entry ADDR]
#
# EFWTOOL defaults to the sibling build tree: set EFWTOOL to override.
set -eu

EFWTOOL="${EFWTOOL:-$(dirname "$0")/../../build/tools/efwtool/efwtool}"

if [ "$#" -lt 2 ]; then
  echo "usage: $0 <payload.bin> <output.img> [--version M.m.p] [--load ADDR] [--entry ADDR]" >&2
  exit 1
fi

payload="$1"; output="$2"; shift 2

# STM32F4 application slot: first 64 KiB of flash (0x08000000-0x0800FFFF)
# is reserved for eBoot; the app image loads at 0x08010000.
load="0x08010000"; entry="0x08010100"; version="0.0.0"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --version) version="$2"; shift 2 ;;
    --load)    load="$2";    shift 2 ;;
    --entry)   entry="$2";   shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 1 ;;
  esac
done

exec "$EFWTOOL" pack "$payload" "$output" \
  --load "$load" --entry "$entry" --version "$version"
