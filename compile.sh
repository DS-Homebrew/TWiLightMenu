#! /bin/sh
mthreads="-j$(nproc)"
cd "booter"
make $mthreads || exit
cp "booter.nds" "../7zfile/BOOT.NDS"
cp "booter.nds" "../7zfile/DSi - CFW users/SDNAND root/title/00030004/53524c41/content/00000000.app"
cd ..
cd "booter_fc"
make $mthreads || exit
cp "booter_fc.nds" "../7zfile/Flashcard users/BOOT.NDS"
cp "booter_fc_cyclodsi.nds" "../7zfile/Flashcard users/BOOT_cyclodsi.NDS"
cd ..
cd "rungame"
make $mthreads || exit
cp "rungame.nds" "../7zfile/DSi - CFW users/SDNAND root/title/00030015/534c524e/content/00000000.app"
cd ..
cd "slot1launch"
make $mthreads || exit
cp "slot1launch.nds" "../7zfile/_nds/TWiLightMenu/slot1launch.srldr"
cd ..
cd "title"
make $mthreads || exit
cp "title.nds" "../7zfile/_nds/TWiLightMenu/main.srldr"
cd ..
cd "settings"
make $mthreads || exit
cp "settings.nds" "../7zfile/_nds/TWiLightMenu/settings.srldr"
cd ..
cd "quickmenu"
make $mthreads || exit
cp "mainmenu.nds" "../7zfile/_nds/TWiLightMenu/mainmenu.srldr"
cd ..
cd "romsel_dsimenutheme"
make $mthreads || exit
cp "romsel_dsimenutheme.nds" "../7zfile/_nds/TWiLightMenu/dsimenu.srldr"
cd ..
cd "romsel_r4theme"
make $mthreads || exit
cp "romsel_r4theme.nds" "../7zfile/_nds/TWiLightMenu/r4menu.srldr"
cd ..
echo -n "Press return to exit..."
read a
