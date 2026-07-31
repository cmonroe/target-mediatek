#!/usr/bin/env python3
"""Alias one FIT image node's payload onto another's.

Rewrites <alias> so it carries data-position/data-size (FIT external-data
accessors, absolute offset) pointing at <source>'s embedded data bytes,
dropping the duplicate copy. Both nodes' payloads must be byte-identical;
per-node hash values stay valid because the referenced bytes are unchanged.

Usage: srg-fit-dedup.py <itb> <alias-image> <source-image>
"""

import struct
import sys

FDT_MAGIC = 0xD00DFEED
FDT_BEGIN_NODE = 0x1
FDT_END_NODE = 0x2
FDT_PROP = 0x3
FDT_NOP = 0x4
FDT_END = 0x9

NEW_STRINGS = b"data-position\0data-size\0"


def die(msg):
    print(f"srg-fit-dedup: error: {msg}", file=sys.stderr)
    sys.exit(1)


def be32_get(blob, off):
    return struct.unpack_from(">I", blob, off)[0]


def pad4(n):
    return (n + 3) & ~3


class fdt_blob:
    def __init__(self, blob):
        if be32_get(blob, 0) != FDT_MAGIC:
            die("bad FDT magic")
        self.blob = blob
        self.totalsize = be32_get(blob, 4)
        self.off_struct = be32_get(blob, 8)
        self.off_strings = be32_get(blob, 12)
        self.off_rsvmap = be32_get(blob, 16)
        self.size_strings = be32_get(blob, 32)
        self.size_struct = be32_get(blob, 36)

    def layout_check(self):
        ordered = self.off_rsvmap < self.off_struct < self.off_strings
        # mkimage leaves slack between the strings block and totalsize
        if not ordered or self.off_strings + self.size_strings > self.totalsize:
            die("unexpected FIT block layout")
        if len(self.blob) != self.totalsize:
            die(f"file size {len(self.blob)} != FDT totalsize {self.totalsize}")

    def strings_end(self):
        return self.off_strings + self.size_strings

    def string_get(self, nameoff):
        base = self.off_strings + nameoff
        end = self.blob.index(b"\0", base)
        return self.blob[base:end].decode()

    def props_walk(self):
        """Yield (path, name, prop_off, val_off, val_len) for every property."""
        blob, off, path = self.blob, self.off_struct, []
        end = self.off_struct + self.size_struct
        while off < end:
            token = be32_get(blob, off)
            if token == FDT_BEGIN_NODE:
                name_end = blob.index(b"\0", off + 4)
                path.append(blob[off + 4:name_end].decode())
                off += 4 + pad4(name_end - (off + 4) + 1)
            elif token == FDT_END_NODE:
                path.pop()
                off += 4
            elif token == FDT_PROP:
                val_len = be32_get(blob, off + 4)
                nameoff = be32_get(blob, off + 8)
                yield ("/" + "/".join(path[1:]), self.string_get(nameoff),
                       off, off + 12, val_len)
                off += 12 + pad4(val_len)
            elif token == FDT_NOP:
                off += 4
            elif token == FDT_END:
                return
            else:
                die(f"bad FDT token {token:#x} at {off:#x}")
        die("FDT_END missing")

    def prop_find(self, path, name):
        for p, n, prop_off, val_off, val_len in self.props_walk():
            if p == path and n == name:
                return prop_off, val_off, val_len
        return None


def prop_build(nameoff, value):
    return struct.pack(">III", FDT_PROP, 4, nameoff) + struct.pack(">I", value)


def main():
    if len(sys.argv) != 4:
        die(f"usage: {sys.argv[0]} <itb> <alias-image> <source-image>")
    path, alias, source = sys.argv[1:4]

    with open(path, "rb") as f:
        blob = f.read()
    fdt = fdt_blob(blob)
    fdt.layout_check()

    src = fdt.prop_find(f"/images/{source}", "data")
    ali = fdt.prop_find(f"/images/{alias}", "data")
    if not src:
        die(f"/images/{source} has no data property")
    if not ali:
        die(f"/images/{alias} has no data property")
    src_off, src_val, src_len = src
    ali_off, ali_val, ali_len = ali
    if blob[src_val:src_val + src_len] != blob[ali_val:ali_val + ali_len]:
        die(f"{alias} and {source} payloads differ, refusing to alias")

    nameoff_pos = fdt.size_strings
    nameoff_size = nameoff_pos + len(b"data-position\0")
    # placeholder 0; the real offset is patched after re-walking the
    # rewritten blob, so no assumption about source-before-alias ordering
    new_props = prop_build(nameoff_pos, 0) + prop_build(nameoff_size, src_len)
    ali_end = ali_val + pad4(ali_len)
    delta = len(new_props) - (ali_end - ali_off)

    out = bytearray()
    out += blob[:ali_off]
    out += new_props
    out += blob[ali_end:fdt.strings_end()]
    out += NEW_STRINGS
    struct.pack_into(">I", out, 4, len(out))
    struct.pack_into(">I", out, 12, fdt.off_strings + delta)
    struct.pack_into(">I", out, 32, fdt.size_strings + len(NEW_STRINGS))
    struct.pack_into(">I", out, 36, fdt.size_struct + delta)

    new_fdt = fdt_blob(bytes(out))
    new_fdt.layout_check()
    src_off, src_val, src_len = new_fdt.prop_find(f"/images/{source}", "data")
    if src_val >= 1 << 32:
        die("source data offset exceeds 32 bits")
    pos_prop = new_fdt.prop_find(f"/images/{alias}", "data-position")
    struct.pack_into(">I", out, pos_prop[1], src_val)

    final = fdt_blob(bytes(out))
    final.layout_check()
    if final.prop_find(f"/images/{alias}", "data"):
        die("alias data property still present after rewrite")
    got_pos = be32_get(out, final.prop_find(f"/images/{alias}", "data-position")[1])
    got_size = be32_get(out, final.prop_find(f"/images/{alias}", "data-size")[1])
    if got_pos != src_val or got_size != src_len:
        die("rewritten data-position/data-size mismatch")

    with open(path, "wb") as f:
        f.write(out)
    print(f"srg-fit-dedup: {alias} -> {source} data at {src_val:#x} "
          f"({src_len} bytes), itb {fdt.totalsize} -> {len(out)} bytes")


if __name__ == "__main__":
    main()
