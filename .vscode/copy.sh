if [ "$1" == "Booter (SD)" ]
	then
	cp $3/booter/booter.nds /Volumes/$2/BOOT.NDS
elif [ "$1" == "Booter (DSiWare)" ]
	then
	cp $3/booter/booter.nds /Volumes/$2/title/00030004/53524c41/content/00000000.app
elif [ "$1" == "Booter (FC)" ]
	then
	cp $3/booter_fc/booter_fc.nds /Volumes/$2/BOOT.NDS
elif [ "$1" == "Main Menu" ]
	then
	cp $3/mainmenu/mainmenu.nds /Volumes/$2/_nds/TWiLightMenu/mainmenu.srldr
elif [ "$1" == "Manual" ]
	then
	cp $3/manual/manual.nds /Volumes/$2/_nds/TWiLightMenu/manual.srldr
elif [ "$1" == "Acekard Theme" ]
	then
	cp $3/romsel_aktheme/romsel_aktheme.nds /Volumes/$2/_nds/TWiLightMenu/akmenu.srldr
elif [ "$1" == "DSi/3DS Theme" ]
	then
	cp $3/romsel_dsimenutheme/romsel_dsimenutheme.nds /Volumes/$2/_nds/TWiLightMenu/dsimenu.srldr
elif [ "$1" == "R4 Theme" ]
	then
	cp $3/romsel_r4theme/romsel_r4theme.nds /Volumes/$2/_nds/TWiLightMenu/r4menu.srldr
elif [ "$1" == "Last Ran Rom" ]
	then
	cp $3/rungame/rungame.nds /Volumes/$2/title/00030015/534c524e/content/00000000.app
elif [ "$1" == "Settings" ]
	then
	cp $3/settings/settings.nds /Volumes/$2/_nds/TWiLightMenu/settings.srldr
elif [ "$1" == "Slot 1 Launch" ]
	then
	cp $3/slot1launch/slot1launch.nds /Volumes/$2/_nds/TWiLightMenu/slot1launch.srldr
elif [ "$1" == "Title" ]
	then
	cp $3/title/title.nds /Volumes/$2/_nds/TWiLightMenu/main.srldr
elif [ "$1" == "All (SD)" ]
	then
	cp $3/booter/booter.nds /Volumes/$2/BOOT.NDS
	cp $3/mainmenu/mainmenu.nds /Volumes/$2/_nds/TWiLightMenu/mainmenu.srldr
	cp $3/manual/manual.nds /Volumes/$2/_nds/TWiLightMenu/manual.srldr
	cp $3/romsel_aktheme/romsel_aktheme.nds /Volumes/$2/_nds/TWiLightMenu/akmenu.srldr
	cp $3/romsel_dsimenutheme/romsel_dsimenutheme.nds /Volumes/$2/_nds/TWiLightMenu/dsimenu.srldr
	cp $3/romsel_r4theme/romsel_r4theme.nds /Volumes/$2/_nds/TWiLightMenu/r4menu.srldr
	cp $3/romsel_r4theme/romsel_r4theme.nds /Volumes/$2/_nds/TWiLightMenu/r4menu.srldr
	cp $3/settings/settings.nds /Volumes/$2/_nds/TWiLightMenu/settings.srldr
	cp $3/slot1launch/slot1launch.nds /Volumes/$2/_nds/TWiLightMenu/slot1launch.srldr
	cp $3/title/title.nds /Volumes/$2/_nds/TWiLightMenu/main.srldr
	cp $3/booter/booter.nds /Volumes/$2/title/00030004/53524c41/content/00000000.app
	cp $3/rungame/rungame.nds /Volumes/$2/title/00030015/534c524e/content/00000000.app
elif [ "$1" == "All (FC)" ]
	then
	cp $3/booter_fc/booter_fc.nds /Volumes/$2/BOOT.NDS
	cp $3/mainmenu/mainmenu.nds /Volumes/$2/_nds/TWiLightMenu/mainmenu.srldr
	cp $3/manual/manual.nds /Volumes/$2/_nds/TWiLightMenu/manual.srldr
	cp $3/romsel_aktheme/romsel_aktheme.nds /Volumes/$2/_nds/TWiLightMenu/akmenu.srldr
	cp $3/romsel_dsimenutheme/romsel_dsimenutheme.nds /Volumes/$2/_nds/TWiLightMenu/dsimenu.srldr
	cp $3/romsel_r4theme/romsel_r4theme.nds /Volumes/$2/_nds/TWiLightMenu/r4menu.srldr
	cp $3/romsel_r4theme/romsel_r4theme.nds /Volumes/$2/_nds/TWiLightMenu/r4menu.srldr
	cp $3/settings/settings.nds /Volumes/$2/_nds/TWiLightMenu/settings.srldr
	cp $3/slot1launch/slot1launch.nds /Volumes/$2/_nds/TWiLightMenu/slot1launch.srldr
	cp $3/title/title.nds /Volumes/$2/_nds/TWiLightMenu/main.srldr
	cp $3/booter/booter.nds /Volumes/$2/title/00030004/53524c41/content/00000000.app
	cp $3/rungame/rungame.nds /Volumes/$2/title/00030015/534c524e/content/00000000.app
else
	echo "Please specify something to copy."
fi
