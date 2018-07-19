#---------------------------------------------------------------------------------
# PACKAGE is the directory where final published files will be placed
#---------------------------------------------------------------------------------
PACKAGE		:=	7zfile

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all pacakge booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme titleandsettings

all:	booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme titleandsettings

package: all
	@mkdir -p "$(PACKAGE)"
	@cp "booter/SRLoader.nds" "$(PACKAGE)/BOOT.NDS"
	@cp "booter_fc/SRLoader.nds" "$(PACKAGE)/BOOT_FC.NDS"

	@mkdir -p "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content"
	@cp "rungame/rungame.nds" "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"

	@mkdir -p "$(PACKAGE)/_nds/dsimenuplusplus"
	@cp "slot1launch/slot1launch.nds" "$(PACKAGE)/_nds/dsimenuplusplus/slot1launch.srldr"
	@cp "titleandsettings/SRLoader.nds" "$(PACKAGE)/_nds/dsimenuplusplus/main.srldr"
	@cp "romsel_dsimenutheme/SRLoader.nds" "$(PACKAGE)/_nds/dsimenuplusplus/dsimenu.srldr"
	@cp "romsel_r4theme/SRLoader.nds" "$(PACKAGE)/_nds/dsimenuplusplus/r4menu.srldr"

booter:
	@$(MAKE) -C booter

booter_fc:
	@$(MAKE) -C booter_fc

rungame:
	@$(MAKE) -C rungame

slot1launch:
	@$(MAKE) -C slot1launch clean
	@$(MAKE) -C slot1launch

romsel_dsimenutheme:
	@$(MAKE) -C romsel_dsimenutheme

romsel_r4theme:
	@$(MAKE) -C romsel_r4theme

titleandsettings:
	@mkdir -p "titleandsettings/arm9/build/"
	@cp "romsel_dsimenutheme/arm9/build/font6x8.h" "titleandsettings/arm9/build/"
	@$(MAKE) -C titleandsettings

clean:
	@echo clean build direcotiries
	@$(MAKE) -C booter clean
	@$(MAKE) -C booter_fc clean
	@$(MAKE) -C rungame clean
	@$(MAKE) -C slot1launch clean
	@$(MAKE) -C titleandsettings clean
	@$(MAKE) -C romsel_dsimenutheme clean
	@$(MAKE) -C romsel_r4theme clean

	@echo clean package files
	@rm "$(PACKAGE)/BOOT.NDS"
	@rm "$(PACKAGE)/BOOT_FC.NDS"
	@rm "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"
	@rm "$(PACKAGE)/_nds/dsimenuplusplus/slot1launch.srldr"
	@rm "$(PACKAGE)/_nds/dsimenuplusplus/main.srldr"
	@rm "$(PACKAGE)/_nds/dsimenuplusplus/dsimenu.srldr"
	@rm "$(PACKAGE)/_nds/dsimenuplusplus/r4menu.srldr"
