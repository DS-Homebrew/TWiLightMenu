#ifndef SAVEMAP_H
#define SAVEMAP_H

int sramlen[] =
{
	0,
	512,
	8192, 65536, 128*1024,
	256*1024, 512*1024, 1024*1024,
	8192*1024, 16384*1024, 65536*1024
};

static const char saveSizeFixList[][4] = {
	"YEE", // Inazuma Eleven
	"BEE", // Inazuma Eleven 2 - Firestorm
	"BEB", // Inazuma Eleven 2 - Blizzard
	"BEZ", // Inazuma Eleven 3 - Bomb Blast
	"BE8", // Inazuma Eleven 3 - Lightning Bolt
	"BOE", // Inazuma Eleven 3 - Team Ogre Attacks!
	"CCU", // Tomodachi Collection
	"AZL", // Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
};

#endif // SAVEMAP_H
