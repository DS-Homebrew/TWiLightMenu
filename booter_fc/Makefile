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
	@cp "$(TARGET)_fc.nds" "../7zfile/Flashcard users/BOOT.NDS"
	@cp "$(TARGET)_cyclodsi.nds" "../7zfile/Flashcard users/BOOT_cyclodsi.NDS"
	@cp "$(TARGET)_fc.arm7.elf" "../7zfile/Debug/$(TARGET)_fc.arm7.elf"
	@cp "$(TARGET)_fc.arm9.elf" "../7zfile/Debug/$(TARGET)_fc.arm9.elf"
	@cp "$(TARGET)_cyclodsi.arm7.elf" "../7zfile/Debug/$(TARGET)_cyclodsi.arm7.elf"
	@cp "$(TARGET)_cyclodsi.arm9.elf" "../7zfile/Debug/$(TARGET)_cyclodsi.arm9.elf"
	@cp "$(TARGET)_scdso.arm7.elf" "../7zfile/Debug/$(TARGET)_scdso.arm7.elf"
	@cp "$(TARGET)_scdso.arm9.elf" "../7zfile/Debug/$(TARGET)_scdso.arm9.elf"

autoboot_dist:	dist
	cp "$(TARGET)_fc.nds" "./flashcart_specifics/booter_fc.nds"
	cd ./flashcart_specifics/

	#### R4 Original & M3 Simply
	cp "booter_fc.nds" "_DS_MENU.nds"
	cp "DLDI/r4_sd.dldi" "r4_sd.dldi"
	dlditool r4_sd.dldi _DS_MENU.nds
	./r4enc _DS_MENU.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"
	mv "_DS_MENU.nds.DAT" "../../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/_DS_MENU.DAT"

	#### R4i Gold 3DS Plus/R4i Gold 3DS Deluxe/R4i Gold 3DS RTS & R4iDSN/R4 Ultra
	##### Global
	cp "booter_fc.nds" "_DS_MENU.nds"
	cp "DLDI/r4idsn_sd.dldi" .
	./dlditool r4idsn_sd.dldi _DS_MENU.nds

	##### R4iDSN & R4 Ultra specific
	cp "_DS_MENU.nds" "../../7zfile/Flashcard users/Autoboot/R4iDSN & R4 Ultra/_MENU_.NDS"
	cp "_DS_MENU.nds" "../../7zfile/Flashcard users/Autoboot/R4iDSN & R4 Ultra/_MENU_B.NDS"

	##### R4i Gold 3DS Plus/R4i Gold 3DS Deluxe/R4i Gold 3DS RTS specific
	./r4enc _DS_MENU.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/"
	mv "_DS_MENU.nds.DAT" "../../7zfile/Flashcard users/Autoboot/R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/_DS_MENU.DAT"

	#### EZ-Flash 5
	cp "booter_fc.nds" "ez5sys.bin"
	cp "DLDI/EZ5V2.dldi" .
	./dlditool EZ5V2.dldi ez5sys.bin
	mkdir "../../7zfile/Flashcard users/Autoboot/EZ Flash V/"
	mv "ez5sys.bin" "../../7zfile/Flashcard users/Autoboot/EZ Flash V/ez5sys.bin"

	#### GBAMP + PassMe, FlashMe or WifiMe
	cp "booter_fc.nds" "_BOOT_MP.NDS"
	cp "DLDI/mpcf.dldi" .
	./dlditool mpcf.dldi _BOOT_MP.NDS
	mkdir "../../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/"
	mv "_BOOT_MP.NDS" "../../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/_BOOT_MP.NDS"

	#### iSmart Premium
	##### research done entirely by devkitPro
	cp "booter_fc.nds" "ismat.dat"
	cp "DLDI/Mat.dldi" .
	./dlditool Mat.dldi ismat.dat
	mkdir "../../7zfile/Flashcard users/Autoboot/iSmart Premium/"
	mkdir "../../7zfile/Flashcard users/Autoboot/iSmart Premium/system/"
	mv "ismat.dat" "../../7zfile/Flashcard users/Autoboot/iSmart Premium/system/ismat.dat"

	#### Acekard 2i/Acekard 2.1 & Galaxy Eagle
	cp "booter_fc.nds" "akmenu4.nds"
	cp "DLDI/ak2_sd.dldi" .
	./dlditool ak2_sd.dldi akmenu4.nds
	cp "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_.NDS"
	cp "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_B.NDS"
	mkdir "../../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/"
	mv "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/akmenu4.nds"

	#### Games N Music
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/"

	##### SDHC but Faster
	cp "booter_fc.nds" "bootme.nds"
	cp "DLDI/gmtf2.dldi" .
	./dlditool gmtf2.dldi bootme.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/"
	mv "bootme.nds" "../../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/bootme.nds"

	##### Not-SDHC restricted but slower
	cp "booter_fc.nds" "bootme.nds"
	cp "DLDI/gmtf.dldi" .
	./dlditool gmtf.dldi bootme.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/"
	mv "bootme.nds" "../../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/bootme.nds"

	#### SuperCard DSTWO
	cp "booter_fc.nds" "dstwo.nds"
	cp "DLDI/dstwo.dldi" .
	./dlditool dstwo.dldi dstwo.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/"
	mkdir "../../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/"
	mv "dstwo.nds" "../../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/dstwo.nds"

	#### SuperCard DSONE
	mkdir "../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/"
	cp "$(TARGET)_scdso.nds" "../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/SCFW.SC"

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