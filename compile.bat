@echo off
cd "booter"
make
copy "SRLoader.nds" "../7zfile/BOOT.NDS"
cd ..
cd "booter_fc"
make
copy "SRLoader.nds" "../7zfile/BOOT_FC.NDS"
cd ..
cd "titleandsettings"
make
copy "SRLoader.nds" "../7zfile/_nds/dsimenuplusplus/main.srldr"
cd ..
cd "romsel_dsimenutheme"
make
copy "SRLoader.nds" "../7zfile/_nds/dsimenuplusplus/dsimenu.srldr"
cd ..
cd "romsel_r4theme"
make
copy "SRLoader.nds" "../7zfile/_nds/dsimenuplusplus/r4menu.srldr"
cd ..
pause