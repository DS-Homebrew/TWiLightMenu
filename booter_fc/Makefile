#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := booter

include $(DEVKITARM)/ds_rules

.PHONY: bootloader bootstub clean makearm7_fc makearm7_cyclodsi makearm7_scdso makearm9_fc makearm9_cyclodsi makearm9_scdso

all:	bootloader bootstub $(TARGET).nds

makearm7_fc:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET)_fc.arm7.elf

makearm7_cyclodsi:
	$(MAKE) -C flashcart_specifics/CycloDSi/arm7
	cp flashcart_specifics/CycloDSi/arm7/$(TARGET).elf $(TARGET)_cyclodsi.arm7.elf

makearm7_scdso:
	$(MAKE) -C flashcart_specifics/DSONE/arm7
	cp flashcart_specifics/DSONE/arm7/$(TARGET).elf $(TARGET)_scdso.arm7.elf

makearm9_fc:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET)_fc.arm9.elf

makearm9_cyclodsi:
	$(MAKE) -C flashcart_specifics/CycloDSi/arm9
	cp flashcart_specifics/CycloDSi/arm9/$(TARGET).elf $(TARGET)_cyclodsi.arm9.elf

makearm9_scdso:
	$(MAKE) -C flashcart_specifics/DSONE/arm9
	cp flashcart_specifics/DSONE/arm9/$(TARGET).elf $(TARGET)_scdso.arm9.elf

dist:	all
	@mkdir "../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/"
	@cp "$(TARGET)_scdso.nds" "../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/SCFW.SC"
	@cp "$(TARGET)_fc.nds" "../7zfile/Flashcard users/BOOT.NDS"
	@cp "$(TARGET)_cyclodsi.nds" "../7zfile/Flashcard users/BOOT_cyclodsi.NDS"
	@cp "$(TARGET)_fc.arm7.elf" "../7zfile/Debug/$(TARGET)_fc.arm7.elf"
	@cp "$(TARGET)_fc.arm9.elf" "../7zfile/Debug/$(TARGET)_fc.arm9.elf"
	@cp "$(TARGET)_cyclodsi.arm7.elf" "../7zfile/Debug/$(TARGET)_cyclodsi.arm7.elf"
	@cp "$(TARGET)_cyclodsi.arm9.elf" "../7zfile/Debug/$(TARGET)_cyclodsi.arm9.elf"
	@cp "$(TARGET)_scdso.arm7.elf" "../7zfile/Debug/$(TARGET)_scdso.arm7.elf"
	@cp "$(TARGET)_scdso.arm9.elf" "../7zfile/Debug/$(TARGET)_scdso.arm9.elf"

$(TARGET).nds:	makearm7_fc makearm7_cyclodsi makearm7_scdso makearm9_fc makearm9_cyclodsi makearm9_scdso
	# simple nds srl without dsi extended header
	ndstool -c $(TARGET)_fc.nds       -7 $(TARGET)_fc.arm7.elf       -9 $(TARGET)_fc.arm9.elf       -h 0x200 -b icon.bmp "TWiLight Menu++;RocketRobz"
	ndstool -c $(TARGET)_cyclodsi.nds -7 $(TARGET)_cyclodsi.arm7.elf -9 $(TARGET)_cyclodsi.arm9.elf -h 0x200 -b icon.bmp "TWiLight Menu++;RocketRobz"

	ndstool -c $(TARGET)_scdso.nds    -7 $(TARGET)_scdso.arm7.elf    -9 $(TARGET)_scdso.arm9.elf    -h ./flashcart_specifics/DSONE/header.bin -t ./flashcart_specifics/DSONE/banner.bin
	dlditool ./flashcart_specifics/DSONE/scsd2.dldi $(TARGET)_scdso.nds

clean:
	@echo clean ...
	@rm -fr data
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET)_fc.nds $(TARGET)_scdso.nds $(TARGET)_cyclodsi.nds
	@rm -fr $(TARGET)_fc.arm7.elf
	@rm -fr $(TARGET)_cyclodsi.arm7.elf
	@rm -fr $(TARGET)_scdso.arm7.elf
	@rm -fr $(TARGET)_fc.arm9.elf
	@rm -fr $(TARGET)_cyclodsi.arm9.elf
	@rm -fr $(TARGET)_scdso.arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C flashcart_specifics/CycloDSi/arm9 clean
	@$(MAKE) -C flashcart_specifics/DSONE/arm9 clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C flashcart_specifics/CycloDSi/arm7 clean
	@$(MAKE) -C flashcart_specifics/DSONE/arm9 clean

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C bootstub