@echo off
make_cia --srl="booter.nds"
copy "booter.cia" "../7zfile/cia/DSiMenuPlusPlus.cia"
pause