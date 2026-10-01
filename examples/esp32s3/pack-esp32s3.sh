#!/bin/sh
# SPDX-License-Identifier: MIT
# Pack a payload binary into an eFirmware image with ESP32-S3 defaults.
#
# Usage: pack-esp32s3.sh <payload.bin> <output.img> [--version M.m.p]
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

# ESP32-S3 factory app partition offset (default ESP-IDF partition table).
load="0x10000"; entry="0x10000"; version="0.0.0"

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
