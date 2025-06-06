#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(PROJECT)),)
export PROJECT	:=	$(CURDIR)/../
endif

export TARGET := imageview
NITRODATA	:=	nitrofiles

include $(DEVKITARM)/ds_rules

.PHONY: bootloader bootstub clean makearm7 makearm9

all:	bootloader bootstub $(TARGET).nds

makearm7:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

makearm9:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

dist:	all
	@mkdir -p ../7zfile/debug
	@cp $(TARGET).nds ../7zfile/Multimedia/_nds/TWiLightMenu/imageview.srldr
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

$(TARGET).nds:	makearm7 makearm9
	ndstool	-u 00030004 -g SRLA 01 "TWLMENUPP" -c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -d $(NITRODATA) \
	-b icon.bmp "Image Viewer;TWiLight Menu++;Rocket Robz"

pages:
	@$(MAKE) -C resources inifiles
	@$(MAKE) -C resources

clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C $(PROJECT)/universal/bootloader_app clean
	@$(MAKE) -C $(PROJECT)/universal/bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C $(PROJECT)/universal/bootloader_app LOADBIN=$(CURDIR)/data/load.bin

bootstub: data
	@$(MAKE) -C $(PROJECT)/universal/bootstub BOOTSTUB=$(CURDIR)/data/bootstub.bin
