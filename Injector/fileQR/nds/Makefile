#---------------------------------------------------------------------------------
# fileQR — receptor NDS (DSi). Projeto dual-CPU (arm7 câmera I2C + arm9 pipeline).
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Defina DEVKITARM no ambiente. export DEVKITARM=<path>/devkitARM")
endif

export TARGET := fileQR
export TOPDIR := $(CURDIR)

GAME_TITLE := fileQR receiver
GAME_AUTHOR := fileQR
GAME_ICON  := icon.bmp

include $(DEVKITARM)/ds_rules

.PHONY: checkarm7 checkarm9 clean

all: checkarm7 checkarm9 $(TARGET).nds

checkarm7:
	$(MAKE) -C arm7

checkarm9:
	$(MAKE) -C arm9

$(TARGET).nds : arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool -c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_AUTHOR)"

arm7/$(TARGET).elf:
	$(MAKE) -C arm7

arm9/$(TARGET).elf:
	$(MAKE) -C arm9

clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds
