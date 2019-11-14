@echo off
cd "booter"
make
copy "booter.nds" "../7zfile/BOOT.NDS"
copy "booter.nds" "../7zfile/DSi - CFW users/SDNAND root/title/00030004/53524c41/content/00000000.app"
cd ..
cd "booter_fc"
make
copy "booter_fc.nds" "../7zfile/Flashcard users/BOOT.NDS"
copy "booter_fc_cyclodsi.nds" "../7zfile/Flashcard users/BOOT_cyclodsi.NDS"
cd ..
cd "rungame"
make
copy "rungame.nds" "../7zfile/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"
cd ..
cd "slot1launch"
make
copy "slot1launch.nds" "../7zfile/_nds/TWiLightMenu/slot1launch.srldr"
cd ..
cd "title"
make
copy "title.nds" "../7zfile/_nds/TWiLightMenu/main.srldr"
cd ..
cd "settings"
make
copy "settings.nds" "../7zfile/_nds/TWiLightMenu/settings.srldr"
cd ..
cd "quickmenu"
make
copy "mainmenu.nds" "../7zfile/_nds/TWiLightMenu/mainmenu.srldr"
cd ..
cd "romsel_dsimenutheme"
make
copy "romsel_dsimenutheme.nds" "../7zfile/_nds/TWiLightMenu/dsimenu.srldr"
cd ..
cd "romsel_r4theme"
make
copy "romsel_r4theme.nds" "../7zfile/_nds/TWiLightMenu/r4menu.srldr"
cd ..
pause