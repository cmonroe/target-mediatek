# SPDX-License-Identifier: GPL-2.0-only
#
# SmartRG combined multi-SoC target: one kernel and one rootfs covering
# MediaTek filogic (MT7622/MT7981/MT7986/MT7987/MT7988) and Airoha AN7581
# SmartRG boards.
#
# Upstream target payloads (patches, file overlays, kernel configs) are
# consumed BY REFERENCE from the pristine in-tree target/linux/mediatek
# and target/linux/airoha directories. This target carries only the
# SmartRG delta on top. See the overrides below for how the stock
# single-PATCH_DIR machinery is extended; everything is local to this
# Makefile, no core include/ changes.

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=cascade
BOARDNAME:=SmartRG multi-SoC (MediaTek Filogic + Airoha AN7581)
SUBTARGETS:=arm64
FEATURES:=dt dt-overlay emmc fpu gpio nand pci pcie pwm ramdisk \
	rootfs-part separate_ramdisk squashfs usb
KERNEL_PATCHVER:=6.18

UPSTREAM_MTK:=$(TOPDIR)/target/linux/mediatek
UPSTREAM_AIROHA:=$(TOPDIR)/target/linux/airoha

# Upstream airoha patches that must not apply on top of the upstream
# mediatek series. 901 carries the identical change as mediatek
# 330-snand-mtk-bmt-support; applying both fails. The others touch
# files the mediatek series also patches (8250_port.c,
# pcie-mediatek-gen3.c), so their hunks land with fuzz or offsets and
# quilt refresh would rewrite them; they are carried rebased in the
# patches-6.18/994-* series instead, keeping the stack fuzz-clean and
# this series byte-stable under refresh.
AIROHA_SKIP:=901-snand-mtk-bmt-support.patch \
	886-uart-add-en7523-support.patch \
	220-10-PCI-mediatek-gen3-set-PHY-mode-for-Airoha-EN7581.patch \
	609-04-PCI-mediatek-gen3-Add-2-lanes-mode-support-for-Airoh.patch

include $(INCLUDE_DIR)/target.mk

FILES_DIR:="$(UPSTREAM_MTK)/files" "$(UPSTREAM_MTK)/files-$(KERNEL_PATCHVER)" "$(CURDIR)/files"

KERNEL_FILE_DEPENDS+=$(UPSTREAM_MTK)/patches-$(KERNEL_PATCHVER) \
	$(UPSTREAM_AIROHA)/patches-$(KERNEL_PATCHVER) \
	$(UPSTREAM_MTK)/files $(UPSTREAM_MTK)/files-$(KERNEL_PATCHVER)

AIROHA_STAGED:=$(KERNEL_BUILD_DIR)/upstream-airoha-filtered

define Kernel/Patch/Default
	$(if $(QUILT),rm -rf $(LINUX_DIR)/patches; mkdir -p $(LINUX_DIR)/patches)
	$(if $(kernel_files),$(CP) $(kernel_files) $(LINUX_DIR)/)
	find $(LINUX_DIR)/ -name \*.rej -or -name \*.orig | $(XARGS) rm -f
	rm -rf $(AIROHA_STAGED)
	mkdir -p $(AIROHA_STAGED)
	$(CP) $(UPSTREAM_AIROHA)/patches-$(KERNEL_PATCHVER)/*.patch $(AIROHA_STAGED)/
	$(foreach p,$(AIROHA_SKIP),rm -f $(AIROHA_STAGED)/$(p);)
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_BACKPORT_DIR),generic-backport/)
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_PATCH_DIR),generic/)
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_HACK_DIR),generic-hack/)
	$(call PatchDir,$(LINUX_DIR),$(UPSTREAM_MTK)/patches-$(KERNEL_PATCHVER),platform/upstream-mediatek/)
	$(call PatchDir,$(LINUX_DIR),$(AIROHA_STAGED),platform/upstream-airoha/)
	$(call PatchDir,$(LINUX_DIR),$(PATCH_DIR),platform/cascade/)
	$(if $(QUILT),touch $(LINUX_DIR)/.quilt_used)
endef
# kernel-defaults.mk snapshotted Kernel/Patch:=$(Kernel/Patch/Default)
# before this Makefile could redefine the body; rebind it.
Kernel/Patch=$(Kernel/Patch/Default)

# Refresh round-trips only the cascade delta. The upstream series are
# read-only references: refreshing them under quilt would silently fork
# them, so fail hard if their patches differ from the reference dirs.
define Quilt/Refresh/Kernel
	@[ -z "$$$$(grep -vE '^(generic(-backport|-hack)?|platform/(upstream-mediatek|upstream-airoha|cascade))/' $(PKG_BUILD_DIR)/patches/series)" ] || { \
		echo "Unexpected series prefix; kernel patches must live in generic*/ or platform/{upstream-mediatek,upstream-airoha,cascade}/"; \
		false; \
	}
	@diff -rq $(PKG_BUILD_DIR)/patches/platform/upstream-mediatek \
		$(UPSTREAM_MTK)/patches-$(KERNEL_PATCHVER) >/dev/null || { \
		echo "ERROR: platform/upstream-mediatek patches were modified under quilt."; \
		echo "Upstream reference patches are read-only; fold the change into platform/cascade/ instead."; \
		false; \
	}
	@diff -rq $(PKG_BUILD_DIR)/patches/platform/upstream-airoha \
		$(AIROHA_STAGED) >/dev/null || { \
		echo "ERROR: platform/upstream-airoha patches were modified under quilt."; \
		echo "Upstream reference patches are read-only; fold the change into platform/cascade/ instead."; \
		false; \
	}
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_BACKPORT_DIR),generic-backport/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_PATCH_DIR),generic/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_HACK_DIR),generic-hack/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR),platform/cascade/)
endef

# Kernel config is likewise composed by reference: upstream filogic and
# an7581 configs merged with kconfig.pl 'm+' (union favoring enabled; a
# later explicit not-set cannot disable the other family's =y), then the
# cascade delta applied last with '+' so it can force symbols off.
CASCADE_KCONFIG_SET = $(GENERIC_LINUX_CONFIG) \
	$(UPSTREAM_MTK)/filogic/config-$(KERNEL_PATCHVER) \
	$(UPSTREAM_AIROHA)/an7581/config-$(KERNEL_PATCHVER) \
	$(LINUX_SUBTARGET_CONFIG)
LINUX_KCONFIG_LIST = $(wildcard $(CASCADE_KCONFIG_SET) $(TOPDIR)/env/kernel-config)
LINUX_CONF_CMD = $(SCRIPT_DIR)/kconfig.pl \
	$(if $(wildcard $(TOPDIR)/env/kernel-config),'+') '+' 'm+' '+' \
	$(CASCADE_KCONFIG_SET) $(wildcard $(TOPDIR)/env/kernel-config)

# kernel_menuconfig / kernel_oldconfig always edit the arm64 delta.
LINUX_RECONFIG_LIST = $(CASCADE_KCONFIG_SET)
LINUX_RECONFIG_TARGET = $(LINUX_SUBTARGET_CONFIG)
LINUX_RECONF_CMD = $(SCRIPT_DIR)/kconfig.pl '+' 'm+' '+' $(CASCADE_KCONFIG_SET)
LINUX_RECONF_DIFF = $(SCRIPT_DIR)/kconfig.pl - '>' 'm+' '+' \
	$(filter-out $(LINUX_RECONFIG_TARGET),$(CASCADE_KCONFIG_SET)) \
	$(1) $(GENERIC_PLATFORM_DIR)/config-filter

DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
