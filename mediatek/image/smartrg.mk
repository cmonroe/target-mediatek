.NOTPARALLEL:

SRGRUN:= TARGET_DIR=$(TARGET_DIR) KDIR=$(KDIR) STAGING_DIR=$(STAGING_DIR_HOST) BIN_DIR=$(BIN_DIR) PACKAGE_DIR=$(PACKAGE_DIR) $(TARGET_DIR)/../flash-images/files/srg-image.sh
BINNAME:=$(IMG_PREFIX)-polecat-root.squashfs
VERNAME:=$(VERSION_NUMBER)-$(subst DEVICE_,,$(PROFILE))

define Device/polecat
  KERNEL_LOADADDR = 0x43200000
  KERNEL_SUFFIX := -fit-multi.itb
  KERNEL_INSTALL := 1
  KERNEL_NAME := Image
  KERNEL = kernel-bin | lzma | SrgFit
  FILESYSTEMS := squashfs
  KERNEL_INITRAMFS :=
  DEVICE_VENDOR := SmartRG
  DEVICE_MODEL := SmartRG Target
  DEVICE_PACKAGES := kmod-hwmon-pwmfan kmod-i2c-gpio kmod-sfp kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk e2fsprogs f2fsck mkf2fs
  DEVICE_DTS := mt7622-smartrg-srbpi
  DEVICE_DTS += mt7622-smartrg-834-5
  DEVICE_DTS += mt7622-smartrg-841-t6
  DEVICE_DTS += mt7622-smartrg-841-t6-mt7531
  DEVICE_DTS += mt7622-smartrg-854-6
  DEVICE_DTS += mt7622-smartrg-854-6-sfp
  DEVICE_DTS += mt7622-smartrg-854-v6
  DEVICE_DTS += mt7622-smartrg-854-v6-sfp
  DEVICE_DTS += mt7622-smartrg-834-v6
  DEVICE_DTS += mt7986a-smartrg-bpi-r3
  DEVICE_DTS += mt7986a-smartrg-SDG-8612
  DEVICE_DTS += mt7986a-smartrg-SDG-8614
  DEVICE_DTS += mt7986a-smartrg-SDG-8622
  DEVICE_DTS += mt7986a-smartrg-SDG-8632
  DEVICE_DTS += mt7981-smartrg-SDG-8610
  DEVICE_DTS += mt7988a-smartrg-SDG-8733
  DEVICE_DTS += mt7988a-smartrg-SDG-8733v
  DEVICE_DTS_DIR := ../dts
  ARTIFACTS := emmc-preloader.bin emmc-bl31-uboot.fip
  ARTIFACT/emmc-preloader.bin := mt7986-bl2 emmc-ddr4
  ARTIFACT/emmc-bl31-uboot.fip := mt7986-bl31-uboot smartrg_bonanza
  DTC_FLAGS += -@
  IMAGES := root.squashfs img img.run
  IMAGE/root.squashfs := SrgDisk
  IMAGE/img := srgImage
  IMAGE/img.run := srgImageRun
endef
TARGET_DEVICES := polecat 
#TARGET_DEVICES += elecom_wrc-2533gent
#TARGET_DEVICES += smartrg_sr402ac
#TARGET_DEVICES += mediatek_mt7622-rfb1

define Build/SrgFit

	srg-mkits.sh -o $@.its -A $(LINUX_KARCH)  -v $(LINUX_VERSION) \
	   	-i "k1" -k $@ -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) -C lzma -h "crc32" -h "sha1" \
		-i "rdisk" -r $(STAGING_DIR_IMAGE)/$(IMG_PREFIX)-initramfs.cpio.gz -h "crc32" -h "sha1" \
		-i "srbpi" -d $(KDIR)/image-mt7622-smartrg-srbpi.dtb -h "crc32" -h "sha1" \
		-i "834-5" -d $(KDIR)/image-mt7622-smartrg-834-5.dtb -h "crc32" -h "sha1" \
		-i "841-t6" -d $(KDIR)/image-mt7622-smartrg-841-t6.dtb -h "crc32" -h "sha1" \
		-i "854-v6" -d $(KDIR)/image-mt7622-smartrg-854-v6.dtb -h "crc32" -h "sha1" \
		-i "854-v6-SFP" -d $(KDIR)/image-mt7622-smartrg-854-v6-sfp.dtb -h "crc32" -h "sha1" \
		-i "834-v6" -d $(KDIR)/image-mt7622-smartrg-834-v6.dtb -h "crc32" -h "sha1" \
		-i "841-t6-mt7531" -d $(KDIR)/image-mt7622-smartrg-841-t6-mt7531.dtb -h "crc32" -h "sha1" \
		-i "854-6" -d $(KDIR)/image-mt7622-smartrg-854-6.dtb -h "crc32" -h "sha1" \
		-i "854-6-SFP" -d $(KDIR)/image-mt7622-smartrg-854-6-sfp.dtb -h "crc32" -h "sha1" \
		-i "srbpi-r3" -d $(KDIR)/image-mt7986a-smartrg-bpi-r3.dtb -h "crc32" -h "sha1" \
		-i "SDG-8612" -d $(KDIR)/image-mt7986a-smartrg-SDG-8612.dtb -h "crc32" -h "sha1" \
		-i "SDG-8614" -d $(KDIR)/image-mt7986a-smartrg-SDG-8614.dtb -h "crc32" -h "sha1" \
		-i "SDG-8622" -d $(KDIR)/image-mt7986a-smartrg-SDG-8622.dtb -h "crc32" -h "sha1" \
		-i "SDG-8632" -d $(KDIR)/image-mt7986a-smartrg-SDG-8632.dtb -h "crc32" -h "sha1" \
		-i "SDG-8610" -d $(KDIR)/image-mt7981-smartrg-SDG-8610.dtb -h "crc32" -h "sha1" \
		-i "SDG-8733" -d $(KDIR)/image-mt7988a-smartrg-SDG-8733.dtb -h "crc32" -h "sha1" \
		-i "SDG-8733v" -d $(KDIR)/image-mt7988a-smartrg-SDG-8733v.dtb -h "crc32" -h "sha1" \
		-c "300" -K k1 -R rdisk -D "srbpi" \
		-c "402" -K k1 -R rdisk -D "834-5" \
		-c "403" -K k1 -R rdisk -D "834-5" \
		-c "404" -K k1 -R rdisk -D "841-t6" \
		-c "405" -K k1 -R rdisk -D "854-v6" \
		-c "405-SFP" -K k1 -R rdisk -D "854-v6-SFP" \
		-c "406" -K k1 -R rdisk -D "854-v6" \
		-c "406-SFP" -K k1 -R rdisk -D "854-v6-SFP" \
		-c "407" -K k1 -R rdisk -D "834-v6" \
		-c "414" -K k1 -R rdisk -D "841-t6-mt7531" \
		-c "415" -K k1 -R rdisk -D "854-6" \
		-c "415-SFP" -K k1 -R rdisk -D "854-6-SFP" \
		-c "416" -K k1 -R rdisk -D "854-6" \
		-c "416-SFP" -K k1 -R rdisk -D "854-6-SFP" \
		-c "302" -K k1 -R rdisk -D "srbpi-r3" \
		-c "420" -K k1 -R rdisk -D "SDG-8612" \
		-c "421" -K k1 -R rdisk -D "SDG-8614" \
		-c "422" -K k1 -R rdisk -D "SDG-8622" \
		-c "423" -K k1 -R rdisk -D "SDG-8632" \
		-c "424" -K k1 -R rdisk -D "SDG-8610" \
		-c "430" -K k1 -R rdisk -D "SDG-8733" \
		-c "431" -K k1 -R rdisk -D "SDG-8733v"

	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv -f $@.new $@

endef

define Build/SrgDiskSquashfs
	@echo "Creating SRG squashfs Image"
	mkdir -p $(TARGET_DIR)/mnt/FLASH
	mkdir -p $(TARGET_DIR)/mnt/boot
	mkdir -p $(TARGET_DIR)/FLASH
	mkdir -p $(TARGET_DIR)/boot
	mkdir -p $(TARGET_DIR)/Boot
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs.run \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT) \
		-processors 1
	$(CP) $(KDIR)/root.squashfs.run $(KDIR)/root.squashfs.run.bin 
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs.run.bin
	sha256sum  $(KDIR)/root.squashfs.run.bin  | cut -d ' ' -f 1 | xargs echo -n  >> $(KDIR)/root.squashfs.run.bin
	$(CP) $(KDIR)/root.squashfs.run.bin $(KDIR)/$(BINNAME).run.bin

	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-polecat-fit-multi.itb $(TARGET_DIR)/Boot/fit-multi.itb
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT) \
		-processors 1
	$(CP) $(KDIR)/root.squashfs $(KDIR)/root.squashfs.bin 
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs.bin
	sha256sum  $(KDIR)/root.squashfs.bin  | cut -d ' ' -f 1 | xargs echo -n  >> $(KDIR)/root.squashfs.bin
	$(CP) $(KDIR)/root.squashfs.bin $(KDIR)/$(BINNAME).bin
	$(CP) $(KDIR)/$(BINNAME).bin $(BIN_DIR)/
endef

define Build/SrgDisk
    $(call Build/SrgDiskSquashfs)
endef

define Build/srgImage
	bash -c "$(SRGRUN) SRGImages $(BINNAME).bin $(VERNAME)" 
endef

define Build/srgImageRun
	rm -rf $(KDIR)/img
	mkdir -p $(KDIR)/img/check_scripts
	mkdir -p $(KDIR)/img/scripts
	mkdir -p $(KDIR)/img/etc
	mkdir -p $(KDIR)/img/cdt
	echo "###### metadata_start ######" > $(KDIR)/img/metadata
	if [ $(1) ]; then \
		CDT_IPK=`find $(wildcard $(PACKAGE_SUBDIRS)) -type f -name 'cdt-$(1)_*.ipk' -print -quit` ; \
		cp $$CDT_IPK $(KDIR)/img/cdt ; \
		echo "CDT=\"$(1)\"" >> $(KDIR)/img/metadata; \
	else \
		echo "CDT= " >> $(KDIR)/img/metadata; \
	fi
	cat $(TARGET_DIR)/etc/openwrt_release | sed "s/'/\"/g" >> $(KDIR)/img/metadata
	cat $(TARGET_DIR)/../flash-images/files/scripts/arch_platforms.sh | grep PLATFORMS >> $(KDIR)/img/metadata
	echo "###### metadata_end ######" >> $(KDIR)/img/metadata
	$(CP) $(TARGET_DIR)/../flash-images/files/scripts/* $(KDIR)/img/check_scripts/
	$(CP) $(TARGET_DIR)/Boot $(KDIR)/img/
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-polecat-fit-multi.itb $(KDIR)/img/Boot/fit-multi.itb
	$(CP) $(TARGET_DIR)/usr/srg/scripts/self-upgrade.sh $(KDIR)/img/
	$(CP) $(TARGET_DIR)/usr/srg/scripts/img.sh $(KDIR)/img/scripts/
	$(CP) $(TARGET_DIR)/usr/srg/scripts/flash-manage.sh $(KDIR)/img/scripts/
	$(CP) $(TARGET_DIR)/usr/srg/scripts/emmc-manage.sh $(KDIR)/img/scripts/
	$(CP) $(TARGET_DIR)/usr/srg/scripts/emmc-disk-* $(KDIR)/img/scripts/
	$(CP) $(TARGET_DIR)/usr/srg/scripts/console_manage.sh $(KDIR)/img/scripts/
	$(CP) $(TARGET_DIR)/etc/openwrt_release $(KDIR)/img/etc/
	$(CP) $(KDIR)/root.squashfs.run.bin $(KDIR)/img/root.squashfs.bin
	tar czf $(KDIR)/$(BINNAME).runimg.tgz -C $(KDIR) img
	$(STAGING_DIR_HOST)/bin/makeself.sh --sha256 --ssl-encrypt --ssl-pass-src file:$(TARGET_DIR)/usr/srg/scripts/pfsos $(KDIR)/img $(KDIR)/$(BINNAME).run "SOS self" ./self-upgrade.sh
	$(CP) $(KDIR)/$(BINNAME).run $(BIN_DIR)/$(BINNAME)$(if $(1),-$(1),).run
	$(CP) $(KDIR)/$(BINNAME).runimg.tgz $(BIN_DIR)/$(BINNAME).runimg.tgz
	@echo "RUNNING BuildPackage alt-os-image"
	mkdir -p $(BIN_DIR)/alt-os-images
	$(STAGING_DIR_HOST)/bin/alt-os-images/transition.sh -f plumeos -t sos -i $(BIN_DIR)/$(BINNAME)$(if $(1),-$(1),).run -d $(BIN_DIR)/alt-os-images
endef

define Image/Flash/mkflash_emmc
	@echo "BUILD FLASH image CDT : $(1)"
	CDT_IPK=`find $(wildcard $(PACKAGE_SUBDIRS)) -type f -name 'cdt-$(1)_*.ipk' -print -quit` ; \
	OUT_DIR="$(BIN_DIR)/flashprogram_bins"; \
    mkdir -p $$OUT_DIR; \
	mk_emmc_mfg_image.sh -e $(2) -r $(TARGET_DIR) -R $(BIN_DIR)/$(BINNAME).bin -b $(BIN_DIR) -c $$CDT_IPK -d $$OUT_DIR/$(IMG_PREFIX)-polecat-emmc-mfg-$(2)-$(1).bin ; \
	cp $(STAGING_DIR_ROOT)/Boot/bin/preloader-emmc.$(2) $$OUT_DIR/$(IMG_PREFIX)-polecat-preloader-emmc-mfg-$(2)-$(1).bin
endef

flashme:
	@echo "Creating FLASH image"
	$(call Image/Flash/mkflash_emmc,$(CDT),$(ENUM))

cdt-image:
	@echo "Build CDT image $(CDT)"
	$(call Build/srgImageRun,$(CDT))
	bash -c "CDT=$(CDT) $(SRGRUN) CDT $(BINNAME) $(VERNAME)"

mini-cdt-image:
	@echo "Build Mini CDT image $(CDT)"
	bash -c "CDT=$(CDT) $(SRGRUN) miniCDT $(BINNAME) $(VERNAME)"

