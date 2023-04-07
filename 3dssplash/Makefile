#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(PROJECT)),)
export PROJECT	:=	$(CURDIR)/../
endif

export TARGET := 3dssplash
NITRODATA	:=	nitrofiles

include $(DEVKITARM)/ds_rules

.PHONY: clean makearm7 makearm9

all:	$(TARGET).nds

makearm7:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

makearm9:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

dist:	all
	@mkdir -p ../7zfile/debug
	@cp $(TARGET).nds ../7zfile/_nds/TWiLightMenu/3dssplash.srldr
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

$(TARGET).nds:	makearm7 makearm9
	ndstool	-u 00030004 -g SRLA 01 "TWLMENUPP" -c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -d $(NITRODATA) \
	-b icon.bmp "Nintendo 3DS Splash;TWiLight Menu++;Rocket Robz"

clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean