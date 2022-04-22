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
	"B7F", // The Magic School Bus: Oceans
	"ARM", // Mario & Luigi: Partners in Time
	"CLJ", // Mario & Luigi: Bowser's Inside Story
	"COL", // Mario & Sonic at the Olympic Winter Games
	"AMM", // Minna no Mahjong DS
	"ARZ", // Rockman ZX/MegaMan ZX
	"YZX", // Rockman ZX Advent/MegaMan ZX Advent
	"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
	"CS3", // Sonic & Sega All-Stars Racing
	"AH9", // Tony Hawk's American Sk8land
	"CTX", // Tropix
};

static const char incompatibleGameListFC[][4] = {
	"CAY", // Army Men: Soldiers of Misfortune
	"YUT", // Ultimate Mortal Kombat
};

static const char incompatibleGameList[][4] = {
	"BO5", // Golden Sun: Dark Dawn
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
	"KDY", // Starship Defense
	"KDZ", // Trajectile
	"KUW", // WarioWare: Snapped!
	"KDX", // X-Scape
};

// B4DS DSiWare Whitelist (Total: 68)
static const char compatibleGameListB4DS[][5] = {
	"KJU", // GO Series: 10 Second Run
	"K95", // 1950s Lawn Mower Kids
	"K99", // 99Bullets
	"K9W", // 99Moves
	"KQK", // Ace Mathician
	"KAA", // Art Style: Aquia
	"KAZ", // ARC Style: Soccer!
	"KAY", // Army Defender
	"KSR", // Aura-Aura Climber
	"KAD", // Art Style: BASE 10
	"KBB", // Bomberman Blitz
	"KCY", // Calculator
	"KC5", // Castle Conqueror: Heroes
	"KCV", // Cave Story
	"KUQ", // Chuck E. Cheese's Alien Defense Force
	"KUC", // Chuck E. Cheese's Arcade Room
	"KXF", // Color Commando
	"KDC", // Crash-Course Domo
	"K32", // CuteWitch! runner
	"KF3", // Dairojo! Samurai Defenders
	"KDV", // Dark Void Zero
	"KWT", // GO Series: Defense Wars
	"KHE", // DotMan
	"KDL", // Dragon's Lair
	"KLYE", // Dragon's Lair II: Time Warp (USA)
	"B88", // DS WiFi Settings
	"Z2E", // Famicom Wars DS: Ushinawareta Hikari
	"KFS", // Flashlight
	"KFP", // Flipper
	"KFG", // Frogger Returns
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
	"KDH", // Hard-Hat Domo
	"K6S", // Heathcliff: Spot On
	"KJY", // JellyCar 2
	"KT9", // Kung Fu Dragon
	"KLK", // Lola's Alphabet Train
	"KJO", // Magnetic Joe
	"KMG", // Mighty Flip Champs!
	"KWY", // Mighty Milky Way
	"KXB", // Monster Buster Club
	"KMB", // Mr. Brain
	"K2D", // Nintendo DSi + Internet
	"KSUE", // Number Battle
	"KAP", // Art Style: PiCTOBiTS
	"KPP", // Pop Island
	"KPF", // Pop Island: Paperfield
	"KOQ", // GO Series: Portable Shrine Wars
	"KAK", // Art Style: precipice
	"KDP", // Pro-Putt Domo
	"KLB", // Rabi Laby
	"KLV", // Rabi Laby 2
	"KD6", // Rock-n-Roll Domo
	"KS3", // Shantae: Risky's Revenge
	"KA6", // Space Ace
	"K4D", // Sudoku
	"K4F", // Sudoku 4Pockets
	"KK4", // Wakugumi: Monochrome Puzzle
	"KDW", // White-Water Domo
	"KAS", // Art Style: ZENGAGE
};

// B4DS DSiWare Whitelist (Show RAM limitation message) (Total: 1)
static const char compatibleGameListB4DSRAMLimited[][4] = {
	"KS3", // Shantae: Risky's Revenge
};

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM) (Total: 16)
static const char compatibleGameListB4DSDebug[][4] = {
	"KXT", // 99Seconds
	"KE9", // Absolute Baseball
	"K27", // G.G. Series: All Breaker
	"KAB", // G.G. Series: Assault Buster
	"KBZ", // BlayzBloo: Super Melee Brawlers Battle Royale
	"K2J", // Cake Ninja
	"KCN", // Castle Conqueror
	"KQN", // Castle Conqueror: Against
	"KXC", // Castle Conqueror: Heroes 2
	"KQN", // Castle Conqueror: Revolution
	"KFD", // Fieldrunners
	"KKN", // Flipper 2: Flush the Goldfish
	"KMM", // Mixed Messages
	"KPS", // Phantasy Star 0 Mini
	"KEV", // Space Invaders Extreme Z
	"KSL", // Touch Solitaire
};

#endif // INCOMPATIBLEMAP_H
