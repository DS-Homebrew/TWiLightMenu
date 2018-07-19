#---------------------------------------------------------------------------------
# PACKAGE is the directory where final published files will be placed
#---------------------------------------------------------------------------------
PACKAGE		:=	7zfile

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all package booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme titleandsettings

all:	booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme titleandsettings

package: all
	@mkdir -p "$(PACKAGE)"
	@cp "booter/booter.nds" "$(PACKAGE)/BOOT.NDS"
	@cp "booter_fc/booter_fc.nds" "$(PACKAGE)/BOOT_FC.NDS"

	@mkdir -p "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content"
	@cp "booter/booter.nds" "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"

	@mkdir -p "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content"
	@cp "rungame/rungame.nds" "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"

	@mkdir -p "$(PACKAGE)/_nds/dsimenuplusplus"
	@cp "slot1launch/slot1launch.nds" "$(PACKAGE)/_nds/dsimenuplusplus/slot1launch.srldr"
	@cp "titleandsettings/titleandsettings.nds" "$(PACKAGE)/_nds/dsimenuplusplus/main.srldr"
	@cp "romsel_dsimenutheme/romsel_dsimenutheme.nds" "$(PACKAGE)/_nds/dsimenuplusplus/dsimenu.srldr"
	@cp "romsel_r4theme/romsel_r4theme.nds" "$(PACKAGE)/_nds/dsimenuplusplus/r4menu.srldr"

booter:
	@$(MAKE) -C booter

booter_fc:
	@$(MAKE) -C booter_fc

rungame:
	@$(MAKE) -C rungame

slot1launch:
	@$(MAKE) -C slot1launch

romsel_dsimenutheme:
	@$(MAKE) -C romsel_dsimenutheme

romsel_r4theme:
	@$(MAKE) -C romsel_r4theme

titleandsettings:
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
	@rm -rf "$(PACKAGE)/BOOT.NDS"
	@rm -rf "$(PACKAGE)/BOOT_FC.NDS"
	@rm -rf "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"
	@rm -rf "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"
	@rm -rf "$(PACKAGE)/_nds/dsimenuplusplus/slot1launch.srldr"
	@rm -rf "$(PACKAGE)/_nds/dsimenuplusplus/main.srldr"
	@rm -rf "$(PACKAGE)/_nds/dsimenuplusplus/dsimenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/dsimenuplusplus/r4menu.srldr"
