#ifndef DONORMAP_H
#define DONORMAP_H

#include <map>
#include <set>

static const std::map<uint, std::set<std::string>> donorMap = { 
	{ 2, {
		"AMQ", // Mario vs. Donkey Kong 2 - March of the Minis
		"AMH", // Metroid Prime Hunters
		"ASM", // Super Mario 64 DS
	}},
	{ 3, {
		"AMC", // Mario Kart DS
		"EKD", // Ermii Kart DS (Mario Kart DS hack)
		"A2D", // New Super Mario Bros.
		"ADA", // Pokemon Diamond
		"APA", // Pokemon Pearl
		"ARZ", // Rockman ZX/MegaMan ZX
		"YZX", // Rockman ZX Advent/MegaMan ZX Advent
	}},
	{ 4, {
		"YKW", // Kirby Super Star Ultra
		"A6C", // MegaMan Star Force: Dragon
		"A6B", // MegaMan Star Force: Leo
		"A6A", // MegaMan Star Force: Pegasus
		"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
		"YT7", // SEGA Superstars Tennis
		"AZL", // Style Savvy
		"BKI", // The Legend of Zelda: Spirit Tracks
		"B3R", // Pokemon Ranger: Guardian Signs
	}},
	{ 5, {
		"B2D", // Doctor Who: Evacuation Earth
		"BH2", // Super Scribblenauts
		"BSD", // Lufia: Curse of the Sinistrals
		"BXS", // Sonic Colo(u)rs
		"BOE", // Inazuma Eleven 3: Sekai heno Chousen! The Ogre
		"BQ8", // Crafting Mama
		"BK9", // Kingdom Hearts: Re-Coded
		"BRJ", // Radiant Historia
		"IRA", // Pokemon Black Version
		"IRB", // Pokemon White Version
		"VI2", // Fire Emblem: Shin Monshou no Nazo Hikari to Kage no Eiyuu
		"BYY", // Yu-Gi-Oh 5Ds World Championship 2011: Over The Nexus
		"UZP", // Learn with Pokemon: Typing Adventure
		"B6F", // LEGO Batman 2: DC Super Heroes
		"IRE", // Pokemon Black Version 2
		"IRD", // Pokemon White Version 2
	}}
};

#endif // DONORMAP_H
