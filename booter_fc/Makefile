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

autoboot_dist:	dist autoboot

.ONESHELL:
autoboot:
	cp "$(TARGET)_fc.nds" "./flashcart_specifics/booter_fc.nds"
	cd flashcart_specifics/

	#### R4 Original & M3 Simply
	cp "booter_fc.nds" "_DS_MENU.nds"
	dlditool ./DLDI/r4_sd.dldi _DS_MENU.nds
	r4denc _DS_MENU.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"
	mv "_DS_MENU.dat" "../../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/_DS_MENU.DAT"

	#### R4i Gold 3DS Plus/R4i Gold 3DS Deluxe/R4i Gold 3DS RTS & R4iDSN/R4 Ultra
	##### Global
	cp "booter_fc.nds" "_DS_MENU.nds"
	dlditool ./DLDI/r4idsn_sd.dldi _DS_MENU.nds

	##### R4iDSN & R4 Ultra specific
	cp "_DS_MENU.nds" "../../7zfile/Flashcard users/Autoboot/R4iDSN & R4 Ultra/_MENU_.NDS"
	mv "_DS_MENU.nds" "../../7zfile/Flashcard users/Autoboot/R4iDSN & R4 Ultra/_MENU_B.NDS"

	##### R4i Gold 3DS Plus/R4i Gold 3DS Deluxe/R4i Gold 3DS RTS specific
	#r4denc _DS_MENU.nds
	#mkdir "../../7zfile/Flashcard users/Autoboot/R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/"
	#mv "_DS_MENU.dat" "../../7zfile/Flashcard users/Autoboot/R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/_DS_MENU.DAT"

	#### EZ-Flash 5
	cp "booter_fc.nds" "ez5sys.bin"
	dlditool ./DLDI/EZ5V2.dldi ez5sys.bin
	mkdir "../../7zfile/Flashcard users/Autoboot/EZ Flash V/"
	mv "ez5sys.bin" "../../7zfile/Flashcard users/Autoboot/EZ Flash V/ez5sys.bin"

	#### GBAMP + PassMe, FlashMe or WifiMe
	cp "booter_fc.nds" "_BOOT_MP.NDS"
	dlditool ./DLDI/mpcf.dldi _BOOT_MP.NDS
	mkdir "../../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/"
	mv "_BOOT_MP.NDS" "../../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/_BOOT_MP.NDS"

	#### iSmart Premium
	##### research done entirely by devkitPro
	cp "booter_fc.nds" "ismat.dat"
	dlditool ./DLDI/Mat.dldi ismat.dat
	mkdir -p "../../7zfile/Flashcard users/Autoboot/iSmart Premium/system/"
	mv "ismat.dat" "../../7zfile/Flashcard users/Autoboot/iSmart Premium/system/ismat.dat"

	#### Acekard 2i/Acekard 2.1 & Galaxy Eagle
	cp "booter_fc.nds" "akmenu4.nds"
	dlditool ./DLDI/ak2_sd.dldi akmenu4.nds
	cp "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_.NDS"
	cp "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_B.NDS"
	mkdir "../../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/"
	mv "akmenu4.nds" "../../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/akmenu4.nds"

	#### DSTT
	cp "booter_fc.nds" "TTMenu.dat"
	dlditool "./DLDI/DSTTDLDIboyakkeyver.dldi" TTMenu.dat

	mkdir "../../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TTMenu.dat"

	mkdir "../../7zfile/Flashcard users/Autoboot/Ace3DS/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/Ace3DS/3DSCARD.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/iLL.iL"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/R4i.dat"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel) & R4i SDHC Silver RTS Lite/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel) & R4i SDHC Silver RTS Lite/R4.dat"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/R4i.3ds"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/R4i.TP"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i King LL/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i King LL/R4KING"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/R4i.TP"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/iLL.iL"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/"
	cp TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/_DS_MENU.DAT"

	# Since this is the last one, move it instead of copy
	mkdir "../../7zfile/Flashcard users/Autoboot/R4i-REDANT/"
	mv TTMenu.dat "../../7zfile/Flashcard users/Autoboot/R4i-REDANT/Redant.dat"

	#### Games N Music
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/"

	##### SDHC but Faster
	cp "booter_fc.nds" "bootme.nds"
	dlditool ./DLDI/gmtf2.dldi bootme.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/"
	mv "bootme.nds" "../../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/bootme.nds"

	##### Not-SDHC restricted but slower
	cp "booter_fc.nds" "bootme.nds"
	dlditool ./DLDI/gmtf.dldi bootme.nds
	mkdir "../../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/"
	mv "bootme.nds" "../../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/bootme.nds"

	#### SuperCard DSTWO
	cp "booter_fc.nds" "dstwo.nds"
	dlditool ./DLDI/dstwo.dldi dstwo.nds
	mkdir -p "../../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/"
	mv "dstwo.nds" "../../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/dstwo.nds"

	#### SuperCard DSONE
	cp "../$(TARGET)_scdso.nds" "SCFW.SC"
	dlditool ./flashcart_specifics/DSONE/scsd2.dldi SCFW.SC
	mkdir "../../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/"
	mv SCFW.SC "../../7zfile/Flashcard users/Autoboot/SuperCard DSONE & SuperCard DSONEi/SCFW.SC"

$(TARGET).nds:	makearm7_fc makearm7_cyclodsi makearm7_scdso makearm9_fc makearm9_cyclodsi makearm9_scdso
	# simple nds srl without dsi extended header
	ndstool -c $(TARGET)_fc.nds       -7 $(TARGET)_fc.arm7.elf       -9 $(TARGET)_fc.arm9.elf       -h 0x200 -b icon.bmp "TWiLight Menu++;RocketRobz"
	ndstool -c $(TARGET)_cyclodsi.nds -7 $(TARGET)_cyclodsi.arm7.elf -9 $(TARGET)_cyclodsi.arm9.elf -h 0x200 -b icon.bmp "TWiLight Menu++;RocketRobz"

	ndstool -c $(TARGET)_scdso.nds    -7 $(TARGET)_scdso.arm7.elf    -9 $(TARGET)_scdso.arm9.elf    -h ./flashcart_specifics/DSONE/header.bin -t ./flashcart_specifics/DSONE/banner.bin

clean:
	@echo clean ...
	@rm -fr data $(BUILD)
	@rm -fr ./flashcart_specifics/booter_fc.nds ./flashcart_specifics/_DS_MENU.nds
	@rm -fr $(TARGET)_fc.nds $(TARGET)_fc.arm7.elf $(TARGET)_fc.arm9.elf
	@rm -fr $(TARGET)_cyclodsi.nds $(TARGET)_cyclodsi.arm7.elf $(TARGET)_cyclodsi.arm9.elf
	@rm -fr $(TARGET)_scdso.nds $(TARGET)_scdso.arm7.elf $(TARGET)_scdso.arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C flashcart_specifics/CycloDSi/arm9 clean
	@$(MAKE) -C flashcart_specifics/DSONE/arm9 clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C flashcart_specifics/CycloDSi/arm7 clean
	@$(MAKE) -C flashcart_specifics/DSONE/arm7 clean

	@echo Cleaning Flashcart Autobooters
	@rm -fr "../../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C bootstub