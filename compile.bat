@echo off
cd "titleandsettings"
make
copy "SRLoader.nds" "../7zfile/BOOT.NDS"
cd ..
cd "romsel_dsmenutheme"
make
copy "SRLoader.nds" "../7zfile/_nds/srloader/dsmenu.srldr"
cd ..
cd "romsel_dsimenutheme"
make
copy "SRLoader.nds" "../7zfile/_nds/srloader/dsimenu.srldr"
cd ..
pause