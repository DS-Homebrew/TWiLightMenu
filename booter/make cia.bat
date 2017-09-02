@echo off
make_cia --srl="SRLoader-DSiWare.nds"
copy "SRLoader-DSiWare.cia" "../7zfile/SRLoader.cia"
pause