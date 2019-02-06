cd "booter"
make
cp "booter.nds" "../7zfile/BOOT.NDS"
cp "booter.nds" "../7zfile/CFW - SDNAND root/title/00030004/53524c41/content/00000000.app"
cd ..
cd "booter_fc"
make
cp "booter_fc.nds" "../7zfile/BOOT_FC.NDS"
cd ..
cd "rungame"
make
cp "rungame.nds" "../7zfile/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"
cd ..
cd "slot1launch"
make
cp "slot1launch.nds" "../7zfile/_nds/TWiLightMenu/slot1launch.srldr"
cd ..
cd "title"
make
cp "title.nds" "../7zfile/_nds/TWiLightMenu/main.srldr"
cd ..
cd "settings"
make
cp "settings.nds" "../7zfile/_nds/TWiLightMenu/settings.srldr"
cd ..
cd "mainmenu"
make
cp "mainmenu.nds" "../7zfile/_nds/TWiLightMenu/mainmenu.srldr"
cd ..
cd "romsel_dsimenutheme"
make
cp "romsel_dsimenutheme.nds" "../7zfile/_nds/TWiLightMenu/dsimenu.srldr"
cd ..
cd "romsel_r4theme"
make
cp "romsel_r4theme.nds" "../7zfile/_nds/TWiLightMenu/r4menu.srldr"
cd ..