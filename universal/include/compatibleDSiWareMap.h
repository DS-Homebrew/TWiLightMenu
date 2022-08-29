#ifndef COMPATIBLEDSIWAREMAP_H
#define COMPATIBLEDSIWAREMAP_H

// B4DS DSiWare Whitelist (Total: 83)
static const char compatibleGameListB4DS[][5] = {
	"KJU", // GO Series: 10 Second Run
	"K95", // 1950s Lawn Mower Kids
	"K99", // 99Bullets
	"K9W", // 99Moves
	"KQK", // Ace Mathician
	"KAC", // Advanced Circuits
	"K5H", // Ah! Heaven
	"KVI", // Anonymous Notes 1: From The Abyss
	"KAA", // Art Style: Aquia
	"KAZ", // ARC Style: Soccer!
	"KAY", // Army Defender
	"KSR", // Aura-Aura Climber
	"KAD", // Art Style: BASE 10
	"KBB", // Bomberman Blitz
	"KAH", // Art Style: Boxlife
	"KKQ", // Bugs'N'Balls
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
	"KD9", // Dr. Mario Express
	"KDL", // Dragon's Lair
	"KLYE", // Dragon's Lair II: Time Warp (USA)
	"B88", // DS WiFi Settings
	"KB8", // GO Series: Earth Saver
	"Z2E", // Famicom Wars DS: Ushinawareta Hikari
	"KU7E", // Fashion Tycoon (USA)
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
	"KWM", // Magical Whip
	"KJO", // Magnetic Joe
	"KMG", // Mighty Flip Champs!
	"KWY", // Mighty Milky Way
	"KXB", // Monster Buster Club
	"KMB", // Mr. Brain
	"K2D", // Nintendo DSi + Internet
	"KSUE", // Number Battle
	"KPJ", // Paul's Shooting Adventure
	"KUS", // Paul's Shooting Adventure 2
	"KAP", // Art Style: PiCTOBiTS
	"KPP", // Pop Island
	"KPF", // Pop Island: Paperfield
	"KOQ", // GO Series: Portable Shrine Wars
	"KAK", // Art Style: precipice
	"KDP", // Pro-Putt Domo
	"KPN", // Puzzle League: Express
	"KUM", // Quick Fill Q
	"KLB", // Rabi Laby
	"KLV", // Rabi Laby 2
	"KRT", // Robot Rescue
	"KD6", // Rock-n-Roll Domo
	"KS3", // Shantae: Risky's Revenge
	"KA6", // Space Ace
	"K4D", // Sudoku
	"K4F", // Sudoku 4Pockets
	"KK4", // Wakugumi: Monochrome Puzzle
	"Z2A", // WarioWare: Touched! DL
	"KDW", // White-Water Domo
	"KAS", // Art Style: ZENGAGE
};

// B4DS DSiWare Whitelist (Show RAM limitation message) (Total: 4)
static const char compatibleGameListB4DSRAMLimited[][4] = {
	"KAA", // Art Style: Aquia
	"KFP", // Flipper
	"KWY", // Mighty Milky Way
	"KS3", // Shantae: Risky's Revenge
};

// B4DS DSiWare Whitelist (RAM limitation message ID)
static int compatibleGameListB4DSRAMLimitedID[] = {
	1, // Art Style: Aquia (No audio)
	2, // Flipper (No music)
	2, // Mighty Milky Way (No music)
	0, // Shantae: Risky's Revenge (One part of the game only)
};

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM) (Show RAM limitation message) (Total: 1)
static const char compatibleGameListB4DSDebugRAMLimited[][4] = {
	"KS3", // Shantae: Risky's Revenge
};

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM) (RAM limitation message ID)
static int compatibleGameListB4DSDebugRAMLimitedID[] = {
	2, // Shantae: Risky's Revenge (No music)
};

// B4DS DSiWare Whitelist (DS Retail & Debug consoles) (Show RAM limitation message) (Total: 1)
static const char compatibleGameListB4DSAllRAMLimited[][4] = {
	"Z2A", // WarioWare: Touched! DL
};

// B4DS DSiWare Whitelist (DS Retail & Debug consoles) (RAM limitation message ID)
static int compatibleGameListB4DSAllRAMLimitedID[] = {
	1, // WarioWare: Touched! DL (No audio)
};

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM) (Total: 23)
static const char compatibleGameListB4DSDebug[][4] = {
	"KII", // 101 Pinball World
	"KXT", // 99Seconds
	"K27", // G.G. Series: All Breaker
	"KAB", // G.G. Series: Assault Buster
	"KBZ", // BlayzBloo: Super Melee Brawlers Battle Royale
	"K2J", // Cake Ninja
	"K2N", // Cake Ninja 2
	"KYN", // Cake Ninja: XMAS
	"KCN", // Castle Conqueror
	"KQN", // Castle Conqueror: Against
	"KXC", // Castle Conqueror: Heroes 2
	"KQN", // Castle Conqueror: Revolution
	"KFD", // Fieldrunners
	"KKN", // Flipper 2: Flush the Goldfish
	"KQ9", // The Legend of Zelda: Four Swords: Anniversary Edition
	"KMM", // Mixed Messages
	"KNP", // Need for Speed: Nitro-X
	"KNV", // Neko Reversi
	"KPS", // Phantasy Star 0 Mini
	"KZL", // Plants vs. Zombies
	"KRR", // Robot Rescue 2
	"KEV", // Space Invaders Extreme Z
	"KSL", // Touch Solitaire
};

#endif // COMPATIBLEDSIWAREMAP_H
