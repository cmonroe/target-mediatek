#!/usr/bin/env bash
# Dry-run the full cascade kernel patch stack into a scratch tree.
# Mirrors the Kernel/Patch/Default override in cascade/Makefile:
#   file overlays -> generic backport/pending/hack -> upstream mediatek
#   -> upstream airoha (minus AIROHA_SKIP) -> cascade delta
#
# Run after every upstream rebase or vendor-drop refresh, before
# committing patch changes.
#
# Usage: stack-check.sh <scratch-dir> [openwrt-topdir]
set -u

SCRATCH="${1:?usage: stack-check.sh <scratch-dir> [openwrt-topdir]}"
TOPDIR="${2:-/sandbox/cmonroe/smartos-combo-v2/polecat/openwrt}"
FEED="$(cd "$(dirname "$0")/.." && pwd)"

GEN="$TOPDIR/target/linux/generic"
UP_MTK="$TOPDIR/target/linux/mediatek"
UP_AIR="$TOPDIR/target/linux/airoha"
CASCADE="$FEED/cascade"
KVER=6.18

# Keep in sync with AIROHA_SKIP in cascade/Makefile.
AIROHA_SKIP="901-snand-mtk-bmt-support.patch
886-uart-add-en7523-support.patch
220-10-PCI-mediatek-gen3-set-PHY-mode-for-Airoha-EN7581.patch
912-pcie-mediatek-gen3-Add-x2-link-support-for-Airoha-EN7581.patch
913-pcie-mediatek-gen3-fix-x2-mode-PERST-deassert.patch"

TARBALL=$(ls "$TOPDIR"/dl/linux-$KVER.*.tar.xz 2>/dev/null | sort -V | tail -1)
[ -n "$TARBALL" ] || { echo "no linux-$KVER tarball in $TOPDIR/dl"; exit 1; }

rm -rf "$SCRATCH"
mkdir -p "$SCRATCH"
echo "extracting $TARBALL ..."
tar -C "$SCRATCH" -xf "$TARBALL"
TREE=$(echo "$SCRATCH"/linux-$KVER.*)
cd "$TREE" || exit 1

echo "copying overlays ..."
cp -a "$UP_MTK/files/." .
cp -a "$UP_MTK/files-$KVER/." .
cp -a "$CASCADE/files/." .

REJLIST="$SCRATCH/rejected-patches.txt"
: > "$REJLIST"
LOGDIR="$SCRATCH/logs"
mkdir -p "$LOGDIR"

apply() {
	local series="$1"; shift
	local p base
	for p in "$@"; do
		base=$(basename "$p")
		if ! patch -p1 -f --no-backup-if-mismatch < "$p" \
			> "$LOGDIR/$base.log" 2>&1; then
			echo "$series $base" >> "$REJLIST"
			echo "REJ [$series] $base"
		fi
	done
}

skip_match() {
	local base="$1"; shift
	local s
	for s in $*; do [ "$base" = "$s" ] && return 0; done
	return 1
}

echo "== generic =="
apply generic-backport "$GEN"/backport-$KVER/*.patch
apply generic-pending "$GEN"/pending-$KVER/*.patch
apply generic-hack "$GEN"/hack-$KVER/*.patch

echo "== upstream mediatek =="
apply upstream-mtk "$UP_MTK"/patches-$KVER/*.patch

echo "== upstream airoha (minus skip) =="
for p in "$UP_AIR"/patches-$KVER/*.patch; do
	skip_match "$(basename "$p")" $AIROHA_SKIP && { echo "skip $(basename "$p")"; continue; }
	apply upstream-airoha "$p"
done

echo "== cascade delta =="
apply cascade-delta "$CASCADE"/patches-$KVER/*.patch

echo
echo "== RESULT =="
if [ -s "$REJLIST" ]; then
	echo "REJECTED PATCHES ($(wc -l < "$REJLIST")):"
	cat "$REJLIST"
	echo
	echo "rej files:"
	find . -name '*.rej' | sort
	exit 1
fi
echo "ALL PATCHES APPLIED CLEAN"
exit 0
