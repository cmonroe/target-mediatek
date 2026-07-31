#!/usr/bin/env bash
# Flag package makefiles that reference the legacy per-family targets
# without also covering TARGET_cascade. CONFIG_TARGET_cascade cannot
# alias the old symbols (they are members of mutually exclusive kconfig
# choice blocks), so union deps must be maintained by hand; this catches
# drift when feeds or vendor drops are refreshed.
#
# Usage: target-dep-lint.sh [repo-root]
set -u

ROOT="${1:-/sandbox/cmonroe/smartos-combo-v2}"
SCAN="$ROOT/shared $ROOT/polecat/openwrt/package"

fail=0
while IFS= read -r file; do
	case "$file" in
	*/target-mediatek/*|*/target-airoha/*|*/target-cascade/*) continue ;;
	*/.git/*|*/build_dir/*|*/staging_dir/*|*/dl/*|*/tmp/*) continue ;;
	esac
	grep -q 'TARGET_cascade' "$file" && continue
	echo "MISSING cascade union: $file"
	grep -n 'TARGET_\(mediatek\|airoha\)' "$file" | head -3 | sed 's/^/    /'
	fail=1
done < <(grep -rl --include=Makefile --include='*.mk' \
	'TARGET_\(mediatek\|airoha\)\([^_a-zA-Z]\|_filogic\|_an7581\|_mt7622\)' \
	$SCAN 2>/dev/null)

[ $fail -eq 0 ] && echo "no drift: all TARGET_mediatek/TARGET_airoha users also cover TARGET_cascade"
exit $fail
