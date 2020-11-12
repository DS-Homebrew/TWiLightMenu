@echo off
make_cia --srl="booter.nds"
make_cia --srl="booter_rungame.nds"
copy "booter.cia" "../7zfile/3DS - CFW users/cia/TWiLight Menu.cia"
copy "booter_rungame.cia" "../7zfile/3DS - CFW users/cia/TWiLight Menu - Game booter.cia"
pause