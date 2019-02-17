#---------------------------------------------------------------------------------
# PACKAGE is the directory where final published files will be placed
#---------------------------------------------------------------------------------
PACKAGE		:=	7zfile

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all package booter booter_fc mainmenu romsel_aktheme romsel_dsimenutheme romsel_r4theme rungame settings slot1launch title

all:	booter booter_fc mainmenu romsel_aktheme romsel_dsimenutheme romsel_r4theme rungame settings slot1launch title

package: all
	@mkdir -p "$(PACKAGE)"
	@cp "booter/booter.nds" "$(PACKAGE)/BOOT.NDS"
	@cp "booter_fc/booter_fc.nds" "$(PACKAGE)/BOOT_FC.NDS"

	@mkdir -p "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030004/53524c41/content"
	@cp "booter/booter.nds" "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030004/53524c41/content/00000000.app"

	@mkdir -p "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/534c524e/content"
	@cp "rungame/rungame.nds" "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"

	@mkdir -p "$(PACKAGE)/_nds/TWiLightMenu"
	@cp "mainmenu/mainmenu.nds" "$(PACKAGE)/_nds/TWiLightMenu/mainmenu.srldr"
	@cp "romsel_aktheme/romsel_aktheme.nds" "$(PACKAGE)/_nds/TWiLightMenu/akmenu.srldr"
	@cp "romsel_dsimenutheme/romsel_dsimenutheme.nds" "$(PACKAGE)/_nds/TWiLightMenu/dsimenu.srldr"
	@cp "romsel_r4theme/romsel_r4theme.nds" "$(PACKAGE)/_nds/TWiLightMenu/r4menu.srldr"
	@cp "settings/settings.nds" "$(PACKAGE)/_nds/TWiLightMenu/settings.srldr"
	@cp "slot1launch/slot1launch.nds" "$(PACKAGE)/_nds/TWiLightMenu/slot1launch.srldr"
	@cp "title/title.nds" "$(PACKAGE)/_nds/TWiLightMenu/main.srldr"

booter:
	@$(MAKE) -C booter

booter_fc:
	@$(MAKE) -C booter_fc

mainmenu:
	@$(MAKE) -C mainmenu

romsel_aktheme:
	@$(MAKE) -C romsel_aktheme

romsel_dsimenutheme:
	@$(MAKE) -C romsel_dsimenutheme

romsel_r4theme:
	@$(MAKE) -C romsel_r4theme

rungame:
	@$(MAKE) -C rungame

settings:
	@$(MAKE) -C settings

slot1launch:
	@$(MAKE) -C slot1launch

title:
	@$(MAKE) -C title

clean:
	@echo clean build directories
	@$(MAKE) -C booter clean
	@$(MAKE) -C booter_fc clean
	@$(MAKE) -C mainmenu clean
	@$(MAKE) -C romsel_aktheme clean
	@$(MAKE) -C romsel_dsimenutheme clean
	@$(MAKE) -C romsel_r4theme clean
	@$(MAKE) -C rungame clean
	@$(MAKE) -C settings clean
	@$(MAKE) -C slot1launch clean
	@$(MAKE) -C title clean

	@echo clean package files
	@rm -rf "$(PACKAGE)/BOOT.NDS"
	@rm -rf "$(PACKAGE)/BOOT_FC.NDS"
	@rm -rf "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/53524c41/content/00000000.app"
	@rm -rf "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/akmenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/dsimenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/main.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/mainmenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/r4menu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/settings.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/slot1launch.srldr"
