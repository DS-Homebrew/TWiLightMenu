#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := manual
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
	@cp $(TARGET).nds ../7zfile/_nds/TWiLightMenu/manual.srldr
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

$(TARGET).nds:	makearm7 makearm9
	ndstool	-u 00030004 -g HBLA -c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf -d $(NITRODATA) \
	-b icon.bmp "TWiLight Menu++;Instruction Manual;RocketRobz, Evie & NightScript"

pages:
	@$(MAKE) -C resources

clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds
	@rm -fr $(TARGET).arm7.elf
	@rm -fr $(TARGET).arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C bootstub
