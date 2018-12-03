#---------------------------------------------------------------------------------
# PACKAGE is the directory where final published files will be placed
#---------------------------------------------------------------------------------
PACKAGE		:=	7zfile

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all package booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme romsel_aktheme title settings

all:	booter booter_fc rungame slot1launch romsel_dsimenutheme romsel_r4theme romsel_aktheme title settings

package: all
	@mkdir -p "$(PACKAGE)"
	@cp "booter/booter.nds" "$(PACKAGE)/BOOT.NDS"
	@cp "booter_fc/booter_fc.nds" "$(PACKAGE)/BOOT_FC.NDS"

	@mkdir -p "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content"
	@cp "booter/booter.nds" "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"

	@mkdir -p "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content"
	@cp "rungame/rungame.nds" "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"

	@mkdir -p "$(PACKAGE)/_nds/TWiLightMenu"
	@cp "slot1launch/slot1launch.nds" "$(PACKAGE)/_nds/TWiLightMenu/slot1launch.srldr"
	@cp "title/title.nds" "$(PACKAGE)/_nds/TWiLightMenu/main.srldr"
	@cp "settings/settings.nds" "$(PACKAGE)/_nds/TWiLightMenu/settings.srldr"
	@cp "romsel_dsimenutheme/romsel_dsimenutheme.nds" "$(PACKAGE)/_nds/TWiLightMenu/dsimenu.srldr"
	@cp "romsel_r4theme/romsel_r4theme.nds" "$(PACKAGE)/_nds/TWiLightMenu/r4menu.srldr"
	@cp "romsel_aktheme/romsel_aktheme.nds" "$(PACKAGE)/_nds/TWiLightMenu/akmenu.srldr"

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

romsel_aktheme:
	@$(MAKE) -C romsel_aktheme

title:
	@$(MAKE) -C title

settings:
	@$(MAKE) -C settings

clean:
	@echo clean build direcotiries
	@$(MAKE) -C booter clean
	@$(MAKE) -C booter_fc clean
	@$(MAKE) -C rungame clean
	@$(MAKE) -C slot1launch clean
	@$(MAKE) -C title clean
	@$(MAKE) -C settings clean
	@$(MAKE) -C romsel_dsimenutheme clean
	@$(MAKE) -C romsel_r4theme clean
	@$(MAKE) -C romsel_aktheme clean

	@echo clean package files
	@rm -rf "$(PACKAGE)/BOOT.NDS"
	@rm -rf "$(PACKAGE)/BOOT_FC.NDS"
	@rm -rf "$(PACKAGE)/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"
	@rm -rf "$(PACKAGE)/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/slot1launch.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/main.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/settings.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/dsimenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/r4menu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/akmenu.srldr"
