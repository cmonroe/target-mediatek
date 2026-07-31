# target-cascade maintenance

The cascade target uses the pristine in-tree `target/linux/mediatek`
and `target/linux/airoha` payloads by reference. These payloads are the
patches, file overlays, and kernel configs. The cascade target then
layers only the SmartRG delta from this feed. All composition logic is
in `cascade/Makefile`; there are no core `include/` changes.

## Kernel tree composition (apply order)

1. file overlays: upstream mediatek `files/` + `files-6.18/`, then
   `cascade/files/` (later wins on collisions)
2. generic backport / pending / hack patches
3. upstream mediatek `patches-6.18` (quilt series `platform/upstream-mediatek/`)
4. upstream airoha `patches-6.18` minus `AIROHA_SKIP`
   (`platform/upstream-airoha/`); the skip list is in
   `cascade/Makefile` and currently drops `901-snand-mtk-bmt-support.patch`
   (identical change already applied by mediatek `330-*`) plus four
   patches whose content is carried rebased in the `994-*` series
5. `cascade/patches-6.18` (`platform/cascade/`), series:
   - `990-*` SmartRG mediatek/cross-subsystem
   - `992-*` Airoha vendor BSP (reserved for the airoha-feed-refresh
     workflow; `files/drivers/net/ethernet/airoha/` arht sources belong
     to the same workflow)
   - `993-*` SmartRG airoha delta carried in step with the
     target-airoha feed (same numbering as there)
   - `994-*` reserved: upstream-airoha patches rebased out via
     AIROHA_SKIP
   - `999-*` MTK SDK backports

## Kernel config composition

`kconfig.pl '+' 'm+' '+' generic filogic an7581 cascade-delta`:
the two upstream family configs merge with `m+`. `m+` is a union that
favors enabled symbols. A later explicit not-set cannot disable the
other family's =y. A later =y overrides =m, and a later =y overrides an
explicit not-set. Then `cascade/arm64/config-6.18` applies last with
`+`, so it can force symbols off. `make kernel_menuconfig` writes back
only the delta.

Regenerating the delta from the old per-family feeds (historical
reference, already done):

    kconfig.pl '>' <upstream filogic config> <old feed filogic config> > delta-mediatek
    kconfig.pl '>' <upstream an7581 config>  <old feed an7581 config>  > delta-airoha
    kconfig.pl '+' delta-airoha delta-mediatek     # mediatek last: DEFAULT_BBR wins

Curated resolutions carried in the delta: BBR default congestion
control (fleet-wide), cpufreq default governor stays USERSPACE
(mediatek fleet parity; AN7581 boards get the performance governor from
`arm64/base-files/etc/init.d/cpufreq-airoha`).

## Refresh gates (run after every upstream rebase or vendor drop)

    tools/stack-check.sh <scratch-dir> [openwrt-topdir]

Apply the full stack into a scratch kernel tree. Any reject is on the
rebase worklist. Escalation path for a new upstream-vs-upstream
conflict:

1. Add the airoha patch to `AIROHA_SKIP`.
2. Re-add its content, rebased, as a `994-*` delta patch.

    tools/target-dep-lint.sh [repo-root]

The linter flags makefiles that reference
`TARGET_mediatek`/`TARGET_airoha` without a `TARGET_cascade` union.
CONFIG_TARGET_cascade cannot alias the old symbols (kconfig
choice-group members), so you maintain the unions by hand. Treat output
as advisory: only packages actually selected in the build config need
fixing.

After a refresh, also do these steps:

1. Diff the resolved kernel `.config` against the previous build.
2. Review the changed symbols.
3. Re-run the dropped-package check (`CONFIG_PACKAGE_*=y|m` diff before
   and after `make defconfig`).
