@echo off
cd "titleandsettings"
make
copy "SRLoader.nds" "../7zfile/_nds/srloader/main.srldr"
cd ..
cd "romsel_dsimenutheme"
make
copy "SRLoader.nds" "../7zfile/_nds/srloader/dsimenu.srldr"
cd ..
pause