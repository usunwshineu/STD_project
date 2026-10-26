#!/usr/bin/env python3
"""App image tools for the FreeRTOS boot flow.

  python mkimage.py app.bin app.img
      Patch size + checksum into the image header so the boot loader can verify it.
      Header (see start.S and ../boot/boot.c):
        +0 j reset | +4 magic "FRTS" | +8 size | +12 checksum
      Checksum = 32-bit sum of all little-endian words, checksum field taken as 0.

  python mkimage.py --flash up5k_riscv.bin app.img up5k_riscv_freertos.bin
      One flash file: bitstream at 0, padded with 0xFF, app.img at 128 KB.
"""
import struct
import sys

MAGIC = 0x53545246
MAX_SIZE = 0xF000          # top 4 KB of SPRAM is the boot loader's stack
APP_FLASH_ADDR = 0x20000   # must match APP_FLASH_ADDR in ../boot/boot.c


def make_image(src: str, dst: str) -> None:
    data = bytearray(open(src, "rb").read())
    data += b"\0" * (-len(data) % 4)                      # pad to whole words

    if struct.unpack_from("<I", data, 4)[0] != MAGIC:
        sys.exit("error: magic not found - is start.S linked first?")
    if len(data) > MAX_SIZE:
        sys.exit(f"error: image is {len(data)} bytes, limit is {MAX_SIZE}")

    struct.pack_into("<II", data, 8, len(data), 0)
    words = struct.unpack(f"<{len(data) // 4}I", data)
    struct.pack_into("<I", data, 12, sum(words) & 0xFFFFFFFF)

    open(dst, "wb").write(data)
    print(f"{dst}: {len(data)} bytes ({100 * len(data) / MAX_SIZE:.0f}% of the {MAX_SIZE} byte limit)")


def make_flash(bitstream: str, img: str, dst: str) -> None:
    bit = open(bitstream, "rb").read()
    app = open(img, "rb").read()
    if len(bit) > APP_FLASH_ADDR:
        sys.exit(f"error: bitstream is {len(bit)} bytes, overlaps the app at {APP_FLASH_ADDR:#x}")
    open(dst, "wb").write(bit + b"\xff" * (APP_FLASH_ADDR - len(bit)) + app)
    print(f"{dst}: bitstream {len(bit)} bytes + app {len(app)} bytes at {APP_FLASH_ADDR:#x}")


if __name__ == "__main__":
    if len(sys.argv) == 3:
        make_image(sys.argv[1], sys.argv[2])
    elif len(sys.argv) == 5 and sys.argv[1] == "--flash":
        make_flash(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        sys.exit(__doc__)
