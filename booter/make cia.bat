@echo off
make_cia --srl="SRLoader.nds"
copy "SRLoader.cia" "../7zfile/cia/DSiMenuPlusPlus.cia"
pause