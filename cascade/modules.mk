# SPDX-License-Identifier: GPL-2.0-only
#
# Union of the mediatek and airoha target kmods, retargeted to
# TARGET_cascade. This target is kernel 6.18 only.

OTHER_MENU:=Other modules

define KernelPackage/ata-ahci-mtk
  TITLE:=Mediatek AHCI Serial ATA support
  KCONFIG:=CONFIG_AHCI_MTK
  FILES:= \
	$(LINUX_DIR)/drivers/ata/ahci_mtk.ko \
	$(LINUX_DIR)/drivers/ata/libahci_platform.ko
  AUTOLOAD:=$(call AutoLoad,40,libahci libahci_platform ahci_mtk,1)
  $(call AddDepends/ata)
  DEPENDS+=@TARGET_cascade
endef

define KernelPackage/ata-ahci-mtk/description
 Mediatek AHCI Serial ATA host controllers
endef

$(eval $(call KernelPackage,ata-ahci-mtk))

define KernelPackage/btmtkuart
  SUBMENU:=Other modules
  TITLE:=MediaTek HCI UART driver
  DEPENDS:=@TARGET_cascade +kmod-bluetooth +kmod-btmtk +mt7622bt-firmware \
	   +kmod-hci-uart
  KCONFIG:=CONFIG_BT_MTKUART
  FILES:= \
	$(LINUX_DIR)/drivers/bluetooth/btmtkuart.ko
  AUTOLOAD:=$(call AutoProbe,btmtkuart)
endef

$(eval $(call KernelPackage,btmtkuart))

define KernelPackage/iio-mt6577-auxadc
  TITLE:=Mediatek AUXADC driver
  DEPENDS:=@TARGET_cascade
  KCONFIG:=CONFIG_MEDIATEK_MT6577_AUXADC
  FILES:= \
	$(LINUX_DIR)/drivers/iio/adc/mt6577_auxadc.ko
  AUTOLOAD:=$(call AutoProbe,mt6577_auxadc)
  $(call AddDepends/iio)
endef
$(eval $(call KernelPackage,iio-mt6577-auxadc))

define KernelPackage/phy-mediatek-2p5g
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MediaTek 2.5G Ethernet PHY
  DEPENDS:=@TARGET_cascade +kmod-libphy
  KCONFIG:=CONFIG_MEDIATEK_2P5GE_PHY
  FILES:= \
   $(LINUX_DIR)/drivers/net/phy/mediatek/mtk-2p5ge.ko
  AUTOLOAD:=$(call AutoLoad,18,mtk-2p5ge,1)
endef

define KernelPackage/phy-mediatek-2p5g/description
  Kernel modules for 2.5G Ethernet PHY built-into the MediaTek MT7988
  and MT7987 SoCs.
endef

$(eval $(call KernelPackage,phy-mediatek-2p5g))

define KernelPackage/switch-rtl8367s
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8367S switch support
  KCONFIG:= \
	CONFIG_RTL8367S_GSW \
	CONFIG_SWCONFIG=y
  DEPENDS:=@TARGET_cascade +kmod-swconfig
  FILES:= \
	$(LINUX_DIR)/drivers/net/phy/rtk/rtl8367s_gsw.ko
  AUTOLOAD:=$(call AutoProbe,rtl8367s_gsw,1)
endef

$(eval $(call KernelPackage,switch-rtl8367s))

I2C_MT7621_MODULES:= \
  CONFIG_I2C_MT7621:drivers/i2c/busses/i2c-mt7621

define KernelPackage/i2c-an7581
  SUBMENU:=$(OTHER_MENU)
  $(call i2c_defaults,$(I2C_MT7621_MODULES),79)
  TITLE:=Airoha I2C Controller
  DEPENDS:=+kmod-i2c-core \
	  @TARGET_cascade
endef

define KernelPackage/i2c-an7581/description
 Kernel modules for enable mt7621 i2c controller.
endef

$(eval $(call KernelPackage,i2c-an7581))

define KernelPackage/pwm-airoha
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Airoha AN7581 PWM
  DEPENDS:=@TARGET_cascade
  KCONFIG:= \
        CONFIG_PWM=y \
        CONFIG_PWM_AIROHA=y \
        CONFIG_PWM_SYSFS=y
  FILES:= \
        $(LINUX_DIR)/drivers/pwm/pwm-airoha.ko
  AUTOLOAD:=$(call AutoProbe,pwm-airoha)
endef

define KernelPackage/pwm-airoha/description
 Kernel module to use the PWM channel on Airoha SoC
endef

$(eval $(call KernelPackage,pwm-airoha))
