ARCH:=aarch64
SUBTARGET:=arm64
BOARDNAME:=SmartRG aarch64 boards (MT7622/MT798x/AN7581)
CPU_TYPE:=cortex-a53
KERNELNAME:=Image dtbs
DEFAULT_PROFILE:=polecat

DEFAULT_PACKAGES += fitblk kmod-crypto-hw-safexcel wpad-basic-mbedtls \
	uboot-envtools airoha-en7581-npu-firmware

define Target/Description
	Single-kernel, single-rootfs image for all SmartRG MediaTek
	(MT7622/MT7981/MT7986/MT7987/MT7988) and Airoha AN7581 boards.
endef
