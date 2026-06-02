@echo off
make_cia --srl="booter.nds"
if not exist "../7zfile/3DS - CFW users/cia" md "../7zfile/3DS - CFW users/cia"
copy "booter.cia" "../7zfile/3DS - CFW users/cia/TWiLight Menu.cia"
pause
