@echo off
make_cia --srl="rungame.nds"
copy "rungame.cia" "../7zfile/cia/DSiMenuPlusPlus - Game booter (NTR touch).cia"
make_cia --srl="rungame-twltouch.nds"
copy "rungame-twltouch.cia" "../7zfile/cia/DSiMenuPlusPlus - Game booter (TWL touch).cia"
pause