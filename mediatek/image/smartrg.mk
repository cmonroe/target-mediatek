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
  DEVICE_PACKAGES := kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk
  DEVICE_DTS := mt7622-smartrg-srbpi
  DEVICE_DTS += mt7622-smartrg-834-5
  DEVICE_DTS += mt7622-smartrg-841-t6
  DEVICE_DTS += mt7622-smartrg-841-t6-mt7531
  DEVICE_DTS += mt7622-smartrg-854-6
  DEVICE_DTS += mt7622-smartrg-854-v6
  DEVICE_DTS += mt7622-smartrg-834-v6
  DEVICE_DTS_DIR := ../dts
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

	mkits-multiple-config.sh -o $@.its -A $(LINUX_KARCH) \
		-v $(LINUX_VERSION) -k $@ -a $(KERNEL_LOADADDR) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-D "k1" -C lzma -c 1 \
		-h "crc32" -h "sha1" \
		-r $(STAGING_DIR_IMAGE)/$(IMG_PREFIX)-initramfs.cpio.gz \
		-D "rdisk" -c 1 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-srbpi.dtb \
		-D "srbpi" -n 300 -c 1 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-834-5.dtb \
		-D "834-5-iPA" -n 402 -c 2 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-834-5.dtb \
		-D "834-5-ePA" -n 403 -c 3 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-841-t6.dtb \
		-D "841-t6" -n 404 -c 4 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-854-v6.dtb \
		-D "854-v6-iPA" -n 405 -c 5 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-854-v6.dtb \
		-D "854-v6-ePA" -n 406 -c 6 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-834-v6.dtb \
		-D "834-v6" -n 407 -c 7 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-841-t6-mt7531.dtb \
		-D "841-t6-mt7531" -n 414 -c 8 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-854-6.dtb \
		-D "854-6-iPA" -n 415 -c 9 \
		-h "crc32" -h "sha1" \
		-d $(KDIR)/image-mt7622-smartrg-854-6.dtb \
		-D "854-6-ePA" -n 416 -c 10 \
		-h "crc32" -h "sha1"
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
	$(call Build/srgImageRun,$(CDT))
	bash -c "CDT=$(CDT) $(SRGRUN) CDT $(BINNAME) $(VERNAME)"

mini-cdt-image:
	@echo "Build Mini CDT image $(CDT)"
	bash -c "CDT=$(CDT) $(SRGRUN) miniCDT $(BINNAME) $(VERNAME)"

