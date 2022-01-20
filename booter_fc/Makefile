#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := booter

include $(DEVKITARM)/ds_rules

.PHONY: bootloader bootstub clean makearm7_fc makearm7_r4ils makearm9_fc makearm9_r4idsn makearm9_acekard2 makearm9_r4igold.cc_wood

all:	bootloader bootstub $(TARGET).nds

makearm7_fc:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET)_fc.arm7.elf

makearm7_r4ils:
	$(MAKE) -C flashcart_specifics/r4ils/arm7
	cp flashcart_specifics/r4ils/arm7/$(TARGET).elf $(TARGET)_r4ils.arm7.elf

makearm9_fc:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET)_fc.arm9.elf

makearm9_r4idsn:
	$(MAKE) -C flashcart_specifics/r4idsn/arm9
	cp flashcart_specifics/r4idsn/arm9/$(TARGET).elf $(TARGET)_r4idsn.arm9.elf

makearm9_acekard2:
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 CRT0=0x02000450
	cp flashcart_specifics/address_adapt/arm9/$(TARGET).elf $(TARGET)_acekard2.arm9.elf
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 clean

makearm9_r4igold.cc_wood:
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 CRT0=0x02000000
	cp flashcart_specifics/address_adapt/arm9/$(TARGET).elf $(TARGET)_r4igold.cc_wood.arm9.elf
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 clean

dist:	all autoboot
	@mkdir -p ../7zfile/debug
	@cp $(TARGET)_fc.nds "../7zfile/Flashcard users/BOOT.NDS"
	@cp $(TARGET)_fc.arm7.elf ../7zfile/debug/$(TARGET)_fc.arm7.elf
	@cp $(TARGET)_fc.arm9.elf ../7zfile/debug/$(TARGET)_fc.arm9.elf
	@cp $(TARGET)_r4ils.arm7.elf ../7zfile/debug/$(TARGET)_r4ils.arm7.elf
	@cp $(TARGET)_r4idsn.arm9.elf ../7zfile/debug/$(TARGET)_r4idsn.arm9.elf
	@cp $(TARGET)_acekard2.arm9.elf ../7zfile/debug/$(TARGET)_acekard2.arm9.elf
	@cp $(TARGET)_r4igold.cc_wood.arm9.elf ../7zfile/debug/$(TARGET)_r4igold.cc_wood.arm9.elf

.ONESHELL:
autoboot:
	#### R4i Gold 3DS Plus/R4i Gold 3DS Deluxe/R4i Gold 3DS RTS & R4iDSN/R4 Ultra
	dlditool flashcart_specifics/DLDI/r4idsn_sd.dldi booter_r4idsn.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4iDSN, R4 Ultra, R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/"
	mv booter_r4idsn.nds "../7zfile/Flashcard users/Autoboot/R4iDSN, R4 Ultra, R4i Gold 3DS Plus, R4i Gold 3DS Deluxe & R4i Gold 3DS RTS/_DS_MENU.DAT"
	
	#### Original R4 / M3 Simply specific
	cp booter_fc.nds _DS_MENU.nds
	dlditool flashcart_specifics/DLDI/r4tfv2.dldi _DS_MENU.nds
	r4denc _DS_MENU.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/_DS_MENU.DAT"
	# delete _DS_MENU.nds for the next cart
	rm -f _DS_MENU.nds
	
	#### Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version specific
	ndstool -c _DS_MENU.nds -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi _DS_MENU.nds
	r4denc -k 0x4002 _DS_MENU.nds _DS_MENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/_DS_MENU.DAT"
	# no longer need _DS_MENU.nds
	rm -f _DS_MENU.nds
	
	ndstool -c _DSMENU.nds -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "####" "##" "R4XX" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi _DSMENU.nds
	r4denc -k 0x4002 _DSMENU.nds _DSMENU.dat
	mv _DSMENU.dat "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/_DSMENU.DAT"
	# delete _DSMENU.nds for next cart
	rm -f _DSMENU.nds
	
	#### Gateway Blue specific
	ndstool -c _DSMENU.nds -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "####" "##" "R4IT" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi _DSMENU.nds
	r4denc -k 0x4002 _DSMENU.nds _DSMENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/Gateway Blue"
	mv _DSMENU.dat "../7zfile/Flashcard users/Autoboot/Gateway Blue/_DSMENU.DAT"
	# no longer need _DSMENU.nds
	rm -f _DSMENU.nds
	
	#### EDGE (TODO: Find out how to encrypt the .dat file)
	# cp booter_fc.nds EDGE.dat
	# dlditool flashcart_specifics/DLDI/EDGEv1.0.dldi EDGE.dat
	# mkdir -p "../7zfile/Flashcard users/Autoboot/EDGE/"
	# mv EDGE.dat "../7zfile/Flashcard users/Autoboot/EDGE/EDGE.dat"

	#### EZ-Flash 5
	cp booter_fc.nds ez5sys.bin
	dlditool flashcart_specifics/DLDI/EZ5V2.dldi ez5sys.bin
	mkdir -p "../7zfile/Flashcard users/Autoboot/EZ Flash V/"
	mv ez5sys.bin "../7zfile/Flashcard users/Autoboot/EZ Flash V/ez5sys.bin"

	#### GBAMP + PassMe, FlashMe or WifiMe
	cp booter_fc.nds _BOOT_MP.NDS
	dlditool flashcart_specifics/DLDI/mpcf.dldi _BOOT_MP.NDS
	mkdir -p "../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/"
	mv _BOOT_MP.NDS "../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/_BOOT_MP.NDS"

	#### iSmart Premium
	##### research done entirely by devkitPro
	cp booter_fc.nds ismat.dat
	dlditool flashcart_specifics/DLDI/Mat.dldi ismat.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/iSmart Premium/system/"
	mv ismat.dat "../7zfile/Flashcard users/Autoboot/iSmart Premium/system/ismat.dat"

	#### Galaxy Eagle
	cp booter_fc.nds ge.nds
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi ge.nds
	cp ge.nds "../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_.NDS"
	cp ge.nds "../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_B.NDS"
	# no longer need ge.nds
	rm -f ge.nds

	#### DSTT
	cp booter_fc.nds TTMenu.dat
	dlditool flashcart_specifics/DLDI/DSTTDLDIboyakkeyver.dldi TTMenu.dat

	mkdir -p "../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/Ace3DS/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Ace3DS/3DSCARD.DAT"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Ace3DS/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/iLL.iL"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/R4i.dat"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel), R4i SDHC Silver RTS Lite, R4iTT v1.6/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel), R4i SDHC Silver RTS Lite, R4iTT v1.6/R4.dat"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel), R4i SDHC Silver RTS Lite, R4iTT v1.6/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/R4i.3ds"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/R4i.TP"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i King LL/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i King LL/R4KING"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i King LL/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/R4i.TP"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/iLL.iL"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/TTMenu.dat"

	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/_BOOT_DS.NDS"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/_DS_MENU.DAT"

	# Since this is the last one, move it instead of copy
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i-REDANT/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-REDANT/Redant.dat"
	mv TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-REDANT/TTMenu.dat"

	#### Games N Music
	mkdir -p "../7zfile/Flashcard users/Autoboot/Games N Music/"

	##### SDHC but Faster
	cp booter_fc.nds bootme.nds
	dlditool flashcart_specifics/DLDI/gmtf2.dldi bootme.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/"
	mv bootme.nds "../7zfile/Flashcard users/Autoboot/Games N Music/SDHC/bootme.nds"

	##### Not-SDHC restricted but slower
	cp booter_fc.nds bootme.nds
	dlditool flashcart_specifics/DLDI/gmtf.dldi bootme.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/"
	mv bootme.nds "../7zfile/Flashcard users/Autoboot/Games N Music/Non-SDHC/bootme.nds"

	#### SuperCard DSTWO
	cp booter_fc.nds dstwo.nds
	dlditool flashcart_specifics/DLDI/dstwo.dldi dstwo.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/"
	mv dstwo.nds "../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/dstwo.nds"

	#### SuperCard DSONE
	ndstool -c SCFW.SC -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "ENG0" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/DSONESDSC.dldi SCFW.SC
	mkdir -p "../7zfile/Flashcard users/Autoboot/SuperCard DSONE/"
	mv SCFW.SC "../7zfile/Flashcard users/Autoboot/SuperCard DSONE/SCFW.SC"

	#### SuperCard DSONE SDHC
	cp booter_fc.nds TTMenu.dat
	dlditool flashcart_specifics/DLDI/DSONESlot-1.dldi TTMenu.dat
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/SuperCard DSONE SDHC & SuperCard DSONEi/Non-SDHC/TTMenu.dat"
	dlditool flashcart_specifics/DLDI/DSTTDLDIboyakkeyver.dldi TTMenu.dat
	mv TTMenu.dat "../7zfile/Flashcard users/Autoboot/SuperCard DSONE SDHC & SuperCard DSONEi/SDHC/TTMenu.dat"

	#### M3 DS Real
	dlditool "flashcart_specifics/DLDI/M3-DS_(SD_Card).dldi" akmenu4.nds
	dsbize akmenu4.nds menu.xx 0x12
	mv menu.xx "../7zfile/Flashcard users/Autoboot/M3 DS Real, M3i Zero (non-GMP-Z003)/SYSTEM/menu.xx"
	cp akmenu4.nds "../7zfile/Flashcard users/Autoboot/M3 DS Real, M3i Zero (non-GMP-Z003)/SRESET.DAT"

	#### Acekard 2, 2.1, 2i
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi akmenu4.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1"
	mv akmenu4.nds "../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/"

	#### R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW cards, r4isdhc.com 2013 cards, r4isdhc.hk cards before 2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+
	cp booter_fc.nds _DS_MENU.dat
	dlditool flashcart_specifics/DLDI/r4itt.dldi _DS_MENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW cards, r4isdhc.com 2013 cards, r4isdhc.hk cards before 2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW cards, r4isdhc.com 2013 cards, r4isdhc.hk cards before 2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+/_DS_MENU.DAT"

	#### Wood R4igold.cc Upgrade
	dlditool flashcart_specifics/DLDI/r4idsn_sd.dldi booter_r4igold.cc_wood.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Wood R4igold.cc Upgrade"
	mv booter_r4igold.cc_wood.nds "../7zfile/Flashcard users/Autoboot/Wood R4igold.cc Upgrade/_DS_MENU.dat"

$(TARGET).nds:	makearm7_fc makearm7_r4ils makearm9_fc makearm9_r4idsn makearm9_acekard2 makearm9_r4igold.cc_wood
	# simple nds srl without dsi extended header
	ndstool -c $(TARGET)_fc.nds              -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_fc.arm9.elf              -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_r4idsn.nds          -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_r4idsn.arm9.elf          -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c akmenu4.nds                   -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_acekard2.arm9.elf        -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_r4igold.cc_wood.nds -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_r4igold.cc_wood.arm9.elf -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	

clean:
	@echo clean ...
	@rm -fr data $(BUILD)
	@rm -fr ./flashcart_specifics/booter_fc.nds ./flashcart_specifics/_DS_MENU.nds
	@rm -fr $(TARGET)_fc.nds $(TARGET)_fc.arm7.elf $(TARGET)_fc.arm9.elf
	@rm -fr $(TARGET)_r4idsn.nds $(TARGET)_r4idsn.arm9.elf
	@rm -fr $(TARGET)_r4idsn.nds $(TARGET)_r4idsn.arm9.elf
	@rm -fr $(TARGET)_r4igold.cc_wood.nds $(TARGET)_r4igold.cc_wood.arm9.elf
	@rm -fr $(TARGET)_r4ils.arm7.elf
	@rm -fr $(TARGET)_acekard2.arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C flashcart_specifics/r4idsn/arm9 clean
	@$(MAKE) -C flashcart_specifics/r4ils/arm7 clean
	@$(MAKE) -C arm7 clean
	# echo Cleaning Flashcart Autobooters
	# rm -fr "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C bootstub
