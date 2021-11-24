#ifndef INCOMPATIBLEMAP_H
#define INCOMPATIBLEMAP_H

static const char incompatibleGameListB4DS[][4] = {
	"ADM", // Animal Crossing: Wild World
	"AQC", // Crayon Shin-chan DS - Arashi o Yobu Nutte Crayoon Daisakusen!
	"YRC", // Crayon Shin-chan - Arashi o Yobu Cinemaland Kachinko Gachinko Daikatsugeki!
	"CL4", // Crayon Shin-Chan - Arashi o Yobu Nendororoon Daihenshin!
	"BQB", // Crayon Shin-chan - Obaka Dainin Den - Susume! Kasukabe Ninja Tai!
	"YKR", // Culdcept DS
	"AWD", // Diddy Kong Racing
	"AK4", // Kabu Trader Shun
	"ARM", // Mario & Luigi: Partners in Time
	"CLJ", // Mario & Luigi: Bowser's Inside Story
	"COL", // Mario & Sonic at the Olympic Winter Games
	"AMM", // Minna no Mahjong DS
	"ARZ", // Rockman ZX/MegaMan ZX
	"YZX", // Rockman ZX Advent/MegaMan ZX Advent
	"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
	"CS3", // Sonic & Sega All-Stars Racing
	"AH9", // Tony Hawk's American Sk8land
};

static const char incompatibleGameListFC[][4] = {
	"CAY", // Army Men: Soldiers of Misfortune
	"YUT", // Ultimate Mortal Kombat
};

static const char incompatibleGameList[][4] = {
	"BO5", // Golden Sun: Dark Dawn
	"APD", // Pokemon Dash
	"A24", // Pokemon Dash (Kiosk Demo)
	"CTX", // Tropix
};

// DSiWare
static const char incompatibleGameListMemoryPit[][4] = {
	"KFZ", // Faceez
	"KGU", // Flipnote Studio
	"KHJ", // Hidden Photo (DSiWare)
	"KQ9", // The Legend of Zelda: Four Swords: Anniversary Edition
	"HNG", // Nintendo DSi Browser
	"KPS", // Phantasy Star 0 Mini
	"KHD", // Sparkle Snapshots
	"KUW", // WarioWare: Snapped!
	"KDX", // X-Scape
};

// B4DS DSiWare Whitelist
static const char compatibleGameListB4DS[][5] = {
	"KJU", // GO Series: 10 Second Run
	"KQK", // Ace Mathician
	"KAA", // Art Style: Aquia
	"KSR", // Aura-Aura Climber
	"KAD", // Art Style: BASE 10
	"KF3", // Dairojo! Samurai Defenders
	"KDV", // Dark Void Zero
	"KDL", // Dragon's Lair
	"KLYE", // Dragon's Lair II: Time Warp (USA)
	"B88", // DS WiFi Settings
	"Z2E", // Famicom Wars DS: Ushinawareta Hikari
	"KWT", // GO Series: Defense Wars
	"KGB", // Game & Watch: Ball
	"KGC", // Game & Watch: Chef
	"KGD", // Game & Watch: Donkey Kong Jr.
	"KGG", // Game & Watch: Flagman
	"KGH", // Game & Watch: Helmet
	"KGJ", // Game & Watch: Judge
	"KGM", // Game & Watch: Manhole
	"KGF", // Game & Watch: Mario's Cement Factory
	"KGV", // Game & Watch: Vermin
	"KGK", // Glory Days: Tactical Defense
	"KMG", // Mighty Flip Champs!
	"K2D", // Nintendo DSi + Internet
	"KPP", // Pop Island
	"KPF", // Pop Island: Paperfield
	"KA6", // Space Ace
};

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM)
static const char compatibleGameListB4DSDebug[][4] = {
	"K27", // G.G. Series: All Breaker
	"KAB", // G.G. Series: Assault Buster
	"KBZ", // BlayzBloo: Super Melee Brawlers Battle Royale
	"KWY", // Mighty Milky Way
};

#endif // INCOMPATIBLEMAP_H
