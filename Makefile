#---------------------------------------------------------------------------------
# PACKAGE is the directory where final published files will be placed
# PROJECT is the root directory of the build system
#---------------------------------------------------------------------------------
PACKAGE		:=	7zfile
export PROJECT	:=	$(CURDIR)

#---------------------------------------------------------------------------------
# Goals for Build
#---------------------------------------------------------------------------------
.PHONY: all package booter booter_fc 3dssplash gbapatcher quickmenu manual resources romsel_aktheme romsel_dsimenutheme romsel_r4theme settings slot1launch title

all:	booter booter_fc 3dssplash gbapatcher quickmenu manual resources romsel_aktheme romsel_dsimenutheme romsel_r4theme settings slot1launch title

package:
	@$(MAKE) -C booter dist
	@$(MAKE) -C booter_fc dist
	@$(MAKE) -C 3dssplash dist
	@$(MAKE) -C gbapatcher dist
	@$(MAKE) -C quickmenu dist
	@$(MAKE) -C manual dist
	@$(MAKE) -C imageview dist
	@$(MAKE) -C resources
	@$(MAKE) -C romsel_aktheme dist
	@$(MAKE) -C romsel_dsimenutheme dist
	@$(MAKE) -C romsel_r4theme dist
	#@$(MAKE) -C rungame dist
	@$(MAKE) -C settings dist
	@$(MAKE) -C slot1launch dist
	@$(MAKE) -C title dist

	@rm -rf 7zfile/*/.gitkeep
	@rm -rf 7zfile/*/*/.gitkeep

booter:
	@$(MAKE) -C booter

booter_fc:
	@$(MAKE) -C booter_fc

3dssplash:
	@$(MAKE) -C 3dssplash

gbapatcher:
	@$(MAKE) -C gbapatcher

quickmenu:
	@$(MAKE) -C quickmenu

manual:
	@$(MAKE) -C manual

imageview:
	@$(MAKE) -C imageview

resources:
	@$(MAKE) -C resources

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
	@$(MAKE) -C 3dssplash clean
	@$(MAKE) -C gbapatcher clean
	@$(MAKE) -C quickmenu clean
	@$(MAKE) -C manual clean
	@$(MAKE) -C imageview clean
	@$(MAKE) -C romsel_aktheme clean
	@$(MAKE) -C romsel_dsimenutheme clean
	@$(MAKE) -C romsel_r4theme clean
	#@$(MAKE) -C rungame clean
	@$(MAKE) -C settings clean
	@$(MAKE) -C slot1launch clean
	@$(MAKE) -C title clean

	@echo clean package files
	@rm -rf "$(PACKAGE)/DSi&3DS - SD card users/BOOT.NDS"
	@rm -rf "$(PACKAGE)/Flashcard users/BOOT.NDS"
	@rm -rf "$(PACKAGE)/Flashcard users/BOOT_cyclodsi.NDS"
	@rm -rf "${PACKAGE}/Flashcard users/Autoboot/Original R4/default.nds"
	@rm -rf "${PACKAGE}/Flashcard users/Autoboot/Original R4/akMenu-Wood UI root/_DS_MENU.DAT"
	@rm -rf "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/53524c41/content/00000000.app"
	@rm -rf "$(PACKAGE)/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/akmenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/3dssplash.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/gbapatcher.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/dsimenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/main.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/mainmenu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/manual.srldr"
	@rm -rf "$(PACKAGE)/Multimedia/_nds/TWiLightMenu/imageview.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/r4menu.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/settings.srldr"
	@rm -rf "$(PACKAGE)/_nds/TWiLightMenu/slot1launch.srldr"
