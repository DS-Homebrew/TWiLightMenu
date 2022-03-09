#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
.SECONDARY:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	slot1launch
export TOPDIR		:=	$(CURDIR)

export VERSION_MAJOR	:= 1
export VERSION_MINOR	:= 99
export VERSTRING	:=	$(VERSION_MAJOR).$(VERSION_MINOR)

.PHONY: cardengine_arm7 bootloader bootloaderAlt clean arm7/$(TARGET).elf arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: cardengine_arm7 bootloader bootloaderAlt $(TARGET).nds

dist:	all
	@mkdir -p ../7zfile/debug
	@cp $(TARGET).nds ../7zfile/_nds/TWiLightMenu/slot1launch.srldr
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

$(TARGET).nds:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf \
			-b $(CURDIR)/icon.bmp "Slot-1 Launcher;TWiLight Menu++;Apache Thunder & Rocket Robz" \
			-g TWL1 01 "TWLMENUPP-S1" -z 80040000 -u 00030004

$(TARGET).arm7	: arm7/$(TARGET).elf
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf
$(TARGET).arm9	: arm9/$(TARGET).elf
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9
	
#---------------------------------------------------------------------------------
cardengine_arm7: data
	@$(MAKE) -C cardengine_arm7

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).nds.orig.nds
	@rm -fr $(TARGET).arm7
	@rm -fr $(TARGET).arm9
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootloaderAlt clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C cardengine_arm7 clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootloaderAlt: data
	@$(MAKE) -C bootloaderAlt
