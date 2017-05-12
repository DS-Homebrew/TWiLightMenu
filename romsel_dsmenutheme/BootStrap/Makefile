#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	bootstrap
export TOPDIR		:=	$(CURDIR)


.PHONY: $(TARGET).arm7.elf $(TARGET).arm9.elf

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: _DS_MENU.DAT ez5sys.bin akmenu4.nds TTMENU.DAT _BOOT_MP.NDS

_DS_MENU.DAT	:	$(TARGET).nds
	@dlditool r4tfv2.dldi $<
	@r4denc $< $@

ez5sys.bin	:	$(TARGET).nds
	@cp $< $@
	@dlditool EZ5V2.dldi $@

akmenu4.nds	:	$(TARGET).nds
	@cp $< $@
	@dlditool ak2_sd.dldi $@

TTMENU.DAT	:	$(TARGET).nds
	@cp $< $@
	@dlditool TTCARDIOLibrary.dldi $@

_BOOT_MP.NDS	:	$(TARGET).nds
	@cp $< $@
	@dlditool mpcf.dldi $@



#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm7.elf $(TARGET).arm9.elf
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7.elf -9 $(TARGET).arm9.elf

#---------------------------------------------------------------------------------
$(TARGET).arm7.elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
$(TARGET).arm9.elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7.elf $(TARGET).arm9.elf _DS_MENU.DAT ez5sys.bin akmenu4.nds TTMENU.DAT _BOOT_MP.NDS

