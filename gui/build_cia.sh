GAME_TITLE="nds-hb-menu"
GAME_SUBTITLE1="built with devkitARM"
GAME_SUBTITLE2="http://devkitpro.org"
GAME_INFO="KHBE 01 TWLHOMEBREW"

cp bootstrap/bootstrap.*.elf .
$DEVKITARM/bin/ndstool	-c bootstrap.nds -7 bootstrap.arm7.elf -9 bootstrap.arm9.elf -g $GAME_INFO -b icon.bmp  "$GAME_TITLE;$GAME_SUBTITLE1;$GAME_SUBTITLE2"  -r9 0x2000000 -r7 0x2380000 -e9 0x2000000 -e7 0x2380000
python patch_ndsheader_dsiware.py --read bootstrap.nds > bootstrap.nds_before_patch_header.txt
python patch_ndsheader_dsiware.py --mode dsi bootstrap.nds
python patch_ndsheader_dsiware.py --read bootstrap.nds > bootstrap.nds_after_patch_header.txt
python patch_ndsheader_dsiware.py --read nds-hb-menu-dsiTest.nds > nds-hb-menu-dsiTest_header.txt

$DEVKITARM/bin/ndstool -c nds-hb-menu.nds -7 default.elf -9 hbmenu.elf -g $GAME_INFO -b icon.bmp  "$GAME_TITLE;$GAME_SUBTITLE1;$GAME_SUBTITLE2"
cp nds-hb-menu.nds nds-hb-menu-dsi.nds

python patch_ndsheader_dsiware.py nds-hb-menu.nds
python patch_ndsheader_dsiware.py --mode dsi nds-hb-menu-dsi.nds 

python patch_ndsheader_dsiware.py --read nds-hb-menu.nds > nds-hb-menu.nds_header.txt

python patch_ndsheader_dsiware.py --read nds-hb-menu-dsi.nds > nds-hb-menu-dsi.nds_header.txt

./make_cia.exe --srl=nds-hb-menu.nds
./make_cia.exe --srl=nds-hb-menu-dsi.nds
./make_cia.exe --srl=bootstrap.nds