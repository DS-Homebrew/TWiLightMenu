@echo off
cd "booter"
make
copy "booter.nds" "../7zfile/BOOT.NDS"
copy "booter.nds" "../7zfile/CFW - SDNAND root/title/00030015/53524c41/content/00000000.app"
cd ..
cd "booter_fc"
make
copy "booter_fc.nds" "../7zfile/BOOT_FC.NDS"
cd ..
cd "rungame"
make
copy "rungame.nds" "../7zfile/CFW - SDNAND root/title/00030015/534c524e/content/00000000.app"
cd ..
cd "slot1launch"
make
copy "slot1launch.nds" "../7zfile/_nds/dsimenuplusplus/slot1launch.srldr"
cd ..
cd "title"
make
copy "title.nds" "../7zfile/_nds/dsimenuplusplus/main.srldr"
cd ..
cd "settings"
make
copy "settings.nds" "../7zfile/_nds/dsimenuplusplus/settings.srldr"
cd ..
cd "romsel_dsimenutheme"
make
copy "romsel_dsimenutheme.nds" "../7zfile/_nds/dsimenuplusplus/dsimenu.srldr"
cd ..
cd "romsel_r4theme"
make
copy "romsel_r4theme.nds" "../7zfile/_nds/dsimenuplusplus/r4menu.srldr"
cd ..
pause