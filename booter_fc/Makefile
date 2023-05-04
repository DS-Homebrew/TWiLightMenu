#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(strip $(PROJECT)),)
export PROJECT	:=	$(CURDIR)/../
endif

ifneq (,$(shell which python3))
PYTHON	:= python3
else ifneq (,$(shell which python2))
PYTHON	:= python2
else ifneq (,$(shell which python))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

export TARGET := booter

include $(DEVKITARM)/ds_rules

.PHONY: bootloader bootstub clean makearm7_fc makearm7_r4ils makearm9_fc makearm9_r4ig makearm9_acekard2 makearm9_r4idsn

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

makearm9_r4ig:
	$(MAKE) -C flashcart_specifics/r4ig/arm9
	cp flashcart_specifics/r4ig/arm9/$(TARGET).elf $(TARGET)_r4ig.arm9.elf

makearm9_acekard2:
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 CRT0=0x02000450
	cp flashcart_specifics/address_adapt/arm9/$(TARGET).elf $(TARGET)_acekard2.arm9.elf
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 clean

makearm9_r4idsn:
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 CRT0=0x02000000
	cp flashcart_specifics/address_adapt/arm9/$(TARGET).elf $(TARGET)_r4idsn.arm9.elf
	$(MAKE) -C flashcart_specifics/address_adapt/arm9 clean

dist:	all autoboot
	@mkdir -p ../7zfile/debug
	@cp $(TARGET)_fc.nds "../7zfile/Flashcard users/BOOT.NDS"
	@cp $(TARGET)_fc.arm7.elf ../7zfile/debug/$(TARGET)_fc.arm7.elf
	@cp $(TARGET)_fc.arm9.elf ../7zfile/debug/$(TARGET)_fc.arm9.elf
	@cp $(TARGET)_r4ils.arm7.elf ../7zfile/debug/$(TARGET)_r4ils.arm7.elf
	@cp $(TARGET)_r4ig.arm9.elf ../7zfile/debug/$(TARGET)_r4ig.arm9.elf
	@cp $(TARGET)_acekard2.arm9.elf ../7zfile/debug/$(TARGET)_acekard2.arm9.elf
	@cp $(TARGET)_r4idsn.arm9.elf ../7zfile/debug/$(TARGET)_r4idsn.arm9.elf

.SILENT: autoboot
autoboot:
	### Special carts first
	#### Acekard 2, 2.1, 2i
	cp booter_acekard2.nds akmenu4.nds
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi akmenu4.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1"
	mv akmenu4.nds "../7zfile/Flashcard users/Autoboot/Acekard 2i & Acekard 2.1/akmenu4.nds"

	#### R4 Ultra, r4ids.cn cards, R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW, r4isdhc.com 2013, r4isdhc.hk -2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+
	cp booter_r4ig.nds _DS_MENU.dat
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi _DS_MENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4 Ultra, r4ids.cn cards, R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW, r4isdhc.com 2013, r4isdhc.hk -2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+/"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/R4 Ultra, r4ids.cn cards, R4iTT 3DS, R4i3D 2012+, r4isdhc.com NEW, r4isdhc.com 2013, r4isdhc.hk -2019, r4igold.cc 3DS translucent, r4i-gold.me 2013+/_DS_MENU.dat"
	
	#### R4iDSN
	cp booter_r4idsn.nds _DSMENU.dat
	dlditool flashcart_specifics/DLDI/r4idsn_sd.dldi _DSMENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4iDSN"
	mv _DSMENU.dat "../7zfile/Flashcard users/Autoboot/R4iDSN/_DSMENU.DAT"

	#### Wood R4igold.cc Upgrade
	cp booter_r4idsn.nds _DS_MENU.dat
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi _DS_MENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/Wood R4igold.cc Upgrade"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/Wood R4igold.cc Upgrade/_DS_MENU.dat"

	#### Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi booter_r4ils.nds
	r4denc -k 0x4002 booter_r4ils.nds _DS_MENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/_DS_MENU.dat"
	
	##### Some carts require gamecode to be R4XX, but are still considered as Ace3DS+ clones
	ndstool -c _DSMENU.nds -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "####" "##" "R4XX" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi _DSMENU.nds
	r4denc -k 0x4002 _DSMENU.nds _DSMENU.dat
	mv _DSMENU.dat "../7zfile/Flashcard users/Autoboot/Ace3DS+, Ace3DS X, R4iLS, r4isdhc.com.cn cards, r4isdhc.hk 2021, R4infinity 2, R4i-XDS 2014 white version/_DSMENU.dat"
	# no longer need _DSMENU.nds
	rm -f _DSMENU.nds
	
	#### Gateway Blue
	ndstool -c _DSMENU.nds -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "####" "##" "R4IT" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/ace3ds_sd.dldi _DSMENU.nds
	r4denc -k 0x4002 _DSMENU.nds _DSMENU.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/Gateway Blue"
	mv _DSMENU.dat "../7zfile/Flashcard users/Autoboot/Gateway Blue/_DSMENU.dat"
	# no longer need _DSMENU.nds
	rm -f _DSMENU.nds
	
	#### SuperCard DSONE
	ndstool -c SCFW.SC -7 $(TARGET)_fc.arm7.elf -9 $(TARGET)_fc.arm9.elf -h 0x200 -g "ENG0" -b icon.bmp "TWiLight Menu++;Rocket Robz"
	dlditool flashcart_specifics/DLDI/scds3.dldi SCFW.SC
	mkdir -p "../7zfile/Flashcard users/Autoboot/SuperCard DSONE/"
	mv SCFW.SC "../7zfile/Flashcard users/Autoboot/SuperCard DSONE/SCFW.SC"
	
	### The following use booter_fc.nds for everything
	#### Original R4 / M3 Simply
	cp booter_fc.nds _DS_MENU.nds
	dlditool flashcart_specifics/DLDI/r4tfv2.dldi _DS_MENU.nds
	r4denc _DS_MENU.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/_DS_MENU.dat"
	# delete _DS_MENU.nds for the next cart
	rm -f _DS_MENU.nds

	# N5
	cp booter_fc.nds _DS_MENU.dat
	dlditool flashcart_specifics/DLDI/r4tf_sdhc.dldi _DS_MENU.dat
	mv _DS_MENU.dat "../7zfile/Flashcard users/Autoboot/N5/_DS_MENU.dat"

	#### EDGE (TODO: Find out how to encrypt the .dat file)
	# cp booter_fc.nds EDGE.dat
	# dlditool flashcart_specifics/DLDI/EDGEv1.0.dldi EDGE.dat
	# mkdir -p "../7zfile/Flashcard users/Autoboot/EDGE/"
	# mv EDGE.dat "../7zfile/Flashcard users/Autoboot/EDGE/EDGE.dat"

	#### EZ-Flash 5
	cp booter_fc.nds ez5sys.bin
	dlditool flashcart_specifics/DLDI/ez5h.dldi ez5sys.bin
	mkdir -p "../7zfile/Flashcard users/Autoboot/EZ Flash V/"
	mv ez5sys.bin "../7zfile/Flashcard users/Autoboot/EZ Flash V/ez5sys.bin"

	#### GBAMP + PassMe, FlashMe or WifiMe
	cp booter_fc.nds _BOOT_MP.NDS
	dlditool flashcart_specifics/DLDI/mpcf.dldi _BOOT_MP.NDS
	mkdir -p "../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/"
	mv _BOOT_MP.NDS "../7zfile/Flashcard users/Autoboot/GBAMP + PassMe, FlashMe or WifiMe/_BOOT_MP.nds"

	#### iSmart Premium
	##### research done entirely by devkitPro
	cp booter_fc.nds ismat.dat
	dlditool flashcart_specifics/DLDI/mati.dldi ismat.dat
	mkdir -p "../7zfile/Flashcard users/Autoboot/iSmart Premium/system/"
	mv ismat.dat "../7zfile/Flashcard users/Autoboot/iSmart Premium/system/ismat.dat"

	#### Galaxy Eagle
	cp booter_fc.nds ge.nds
	dlditool flashcart_specifics/DLDI/ak2_sd.dldi ge.nds
	cp ge.nds "../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_.nds"
	cp ge.nds "../7zfile/Flashcard users/Autoboot/Galaxy Eagle/_MENU_B.nds"
	# no longer need ge.nds
	rm -f ge.nds

	#### M3 DS Real, iTouchDS, R4(i) SDHC (r4rts.com)
	cp booter_fc.nds r4i.sys
	dlditool "flashcart_specifics/DLDI/m3ds.dldi" r4i.sys
	mkdir -p "../7zfile/Flashcard users/Autoboot/M3 DS Real, M3i Zero (non-GMP-Z003), iTouchDS, R4(i) SDHC (r4rts.com)/_system_/_sys_data"
	cp r4i.sys "../7zfile/Flashcard users/Autoboot/M3 DS Real, M3i Zero (non-GMP-Z003), iTouchDS, R4(i) SDHC (r4rts.com)/_system_/_sys_data/r4i.sys"
	mv r4i.sys "../7zfile/Flashcard users/Autoboot/M3 DS Real, M3i Zero (non-GMP-Z003), iTouchDS, R4(i) SDHC (r4rts.com)/SRESET.dat"

	#### SuperCard DSONE SDHC / DSONEi
	cp booter_fc.nds TTMenu.dat
	dlditool flashcart_specifics/DLDI/scdssdhc2.dldi TTMenu.dat
	mv TTMenu.dat "../7zfile/Flashcard users/Autoboot/SuperCard DSONE SDHC & SuperCard DSONEi/TTMenu.dat"

	#### SuperCard DSTWO
	cp booter_fc.nds dstwo.nds
	dlditool flashcart_specifics/DLDI/dstwo.dldi dstwo.nds
	mkdir -p "../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/"
	mv dstwo.nds "../7zfile/Flashcard users/Autoboot/SuperCard DSTWO/_dstwo/dstwo.nds"

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

	#### DSTT and its (many) clones
	cp booter_acekard2.nds TTMenu.dat
	dlditool flashcart_specifics/DLDI/ttio_sdhc.dldi TTMenu.dat

	mkdir -p "../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/Ace3DS/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Ace3DS/3DSCARD.dat"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Ace3DS/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, R4i 3D (www.r4i-gold.hk), GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, R4i 3D (www.r4i-gold.hk), GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/iLL.iL"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, R4i 3D (www.r4i-gold.hk), GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/TTMenu.dat"

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

	# R4i-SDHC has funny protection. Defeat it.
	ndstool -x TTMenu.dat -9 arm9.bin
	$(PYTHON) flashcart_specifics/r4isdhc/r4isdhc.py arm9.bin new9.bin
	ndstool -c R4.dat -9 new9.bin -7 $(TARGET)_fc.arm7.elf -r9 0x02000000 -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	rm -rf new9.bin arm9.bin arm7.bin banner.bin header.bin
	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards 2014+, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/"
	mv R4.dat "../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards 2014+, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/R4.dat"
	## For YSMenu softreset, use the original TTMenu.dat
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards 2014+, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/TTMenu.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/_DS_MENU.dat"

	mkdir -p "../7zfile/Flashcard users/Autoboot/R4i-REDANT/"
	cp TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-REDANT/Redant.dat"
	# Since this is the last one, move it instead of copy
	mv TTMenu.dat "../7zfile/Flashcard users/Autoboot/R4i-REDANT/TTMenu.dat"


$(TARGET).nds:	makearm7_fc makearm7_r4ils makearm9_fc makearm9_r4ig makearm9_acekard2 makearm9_r4idsn
	# simple nds srl without dsi extended header
	ndstool -c $(TARGET)_fc.nds       -7 $(TARGET)_fc.arm7.elf    -9 $(TARGET)_fc.arm9.elf       -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_r4ig.nds     -7 $(TARGET)_fc.arm7.elf    -9 $(TARGET)_r4ig.arm9.elf     -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_r4idsn.nds   -7 $(TARGET)_fc.arm7.elf    -9 $(TARGET)_r4idsn.arm9.elf   -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_acekard2.nds -7 $(TARGET)_fc.arm7.elf    -9 $(TARGET)_acekard2.arm9.elf -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	ndstool -c $(TARGET)_r4ils.nds    -7 $(TARGET)_r4ils.arm7.elf -9 $(TARGET)_fc.arm9.elf       -h 0x200 -b icon.bmp "TWiLight Menu++;Rocket Robz"
	

clean:
	@echo clean ...
	@rm -fr data $(BUILD)
	@rm -fr ./flashcart_specifics/booter_fc.nds ./flashcart_specifics/_DS_MENU.nds
	@rm -fr $(TARGET)_fc.nds $(TARGET)_fc.arm7.elf $(TARGET)_fc.arm9.elf
	@rm -fr $(TARGET)_r4ig.nds $(TARGET)_r4ig.arm9.elf
	@rm -fr $(TARGET)_r4idsn.nds $(TARGET)_r4idsn.arm9.elf
	@rm -fr $(TARGET)_acekard2.nds $(TARGET)_acekard2.arm9.elf
	@rm -fr $(TARGET)_r4ils.nds $(TARGET)_r4ils.arm7.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C $(PROJECT)/universal/bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C flashcart_specifics/r4ig/arm9 clean
	@$(MAKE) -C flashcart_specifics/r4ils/arm7 clean
	@$(MAKE) -C arm7 clean
	# echo Cleaning Flashcart Autobooters
	# rm -fr "../7zfile/Flashcard users/Autoboot/Original R4 & M3 Simply/"

data:
	@mkdir -p data

bootloader: data
	@$(MAKE) -C bootloader

bootstub: data
	@$(MAKE) -C $(PROJECT)/universal/bootstub BOOTSTUB=$(CURDIR)/data/bootstub.bin
