#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := rebooter

include $(DEVKITARM)/ds_rules

.PHONY: clean makearm7_fc makearm9_fc

all:	$(TARGET).nds

makearm7_fc:
	$(MAKE) -C arm7
	cp arm7/$(TARGET).elf $(TARGET).arm7.elf

makearm9_fc:
	$(MAKE) -C arm9
	cp arm9/$(TARGET).elf $(TARGET).arm9.elf

dist:	all
	@cp $(TARGET).nds "../../7zfile/Flashcard users/Autoboot/R4i-SDHC, r4isdhc.com cards, R4i SDHC Upgrade Revolution, R4DSiXL3D, R4i Advance, R4-IIIi, R4 SDHC Revolution, R4(i) Pocket, R4i Gold (v1.4.1) (3DS) & R4xDS/TWLMRS.DAT"
	@cp $(TARGET).arm7.elf ../7zfile/debug/$(TARGET).arm7.elf
	@cp $(TARGET).arm9.elf ../7zfile/debug/$(TARGET).arm9.elf

autoboot_dist:	dist autoboot

.ONESHELL:
autoboot:
	cp "rebooter.nds" "flashcart_specifics/TTMenu/TWLMRS.DAT"
	cd flashcart_specifics/TTMenu/

	#### DSTT
	mkdir "../../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/DSTT, DSTTi, DSTTi Gold, DSTT-Advance, R4Top Revolution, & R4i-SDHC v1.41 + v1.42/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/Ace3DS/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/Ace3DS/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/Ace3DS/3DSCARD.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/Blue R4i Revolution v1.4.1, R4i Gold Upgrade Revolution v1.4.1, GoldR4 3DS (v4.301 kernel) & R4i SDHC Upgrade Revolution (www.r4i-sdhc.com.tw)/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i Gold Upgrade Revolution (v1.14b kernel)/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel) & R4i SDHC Silver RTS Lite/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel) & R4i SDHC Silver RTS Lite/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4IIISDHC (v3.07 kernel) & R4i SDHC Silver RTS Lite/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i Upgrade Revolution/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i SDHC Upgrade Revolution (www.r4i-dshc.com), R4i 3DS (v4.3 kernel) & R4i YES/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i King LL/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4i King LL/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i King LL/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i DSi XL & R4V-R4i v2.2 + v2.5/TWLMRS.DAT"

	mkdir "../../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/TTMenu/"
	cp ttreset.dat "../../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/TTMenu/ttreset.dat"
	cp TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4 Deluxe v1.20/TWLMRS.DAT"

	# Since this is the last one, move it instead of copy
	mkdir "../../7zfile/Flashcard users/Autoboot/R4i-REDANT/"
	mv TWLMRS.DAT "../../7zfile/Flashcard users/Autoboot/R4i-REDANT/TWLMRS.DAT"

$(TARGET).nds:	makearm7_fc makearm9_fc
	# simple nds srl without dsi extended header
	ndstool -c $(TARGET).nds       -7 $(TARGET).arm7.elf       -9 $(TARGET).arm9.elf

clean:
	@echo clean ...@rm -fr data $(BUILD)
	@rm -fr $(TARGET).nds $(TARGET).arm7.elf $(TARGET).arm9.elf
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootstub clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C arm7 clean
