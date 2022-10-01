#ifndef COMPATIBLEDSIWAREMAP_H
#define COMPATIBLEDSIWAREMAP_H

// B4DS DSiWare Whitelist (Total: 106)
static const char compatibleGameListB4DS[][5] = {
	"KJU", // GO Series: 10 Second Run
	"K95", // 1950s Lawn Mower Kids
	"K45", // 40-in-1: Explosive Megamix
	"K99", // 99Bullets
	"K9W", // 99Moves
	"KQK", // Ace Mathician
	"KAC", // Advanced Circuits
	"K5H", // Ah! Heaven
	"KF2", // Amakuchi! Dairoujou
	"KY8", // Anne's Doll Studio: Antique Collection
	"K54", // Anne's Doll Studio: Gothic Collection
	"KLQ", // Anne's Doll Studio: Lolita Collection
	"K2S", // Anne's Doll Studio: Princess Collection
	"KSQ", // Anne's Doll Studio: Tokyo Collection
	"KVI", // Anonymous Notes 1: From The Abyss
	"KV2", // Anonymous Notes 2: From The Abyss
	"KV3", // Anonymous Notes 3: From The Abyss
	"KV4", // Anonymous Notes 4: From The Abyss
	"KAA", // Art Style: Aquia
	"KAZ", // ARC Style: Soccer!
	"KAY", // Army Defender
	"KSR", // Aura-Aura Climber
	"KAD", // Art Style: BASE 10
	"K8B", // Beauty Academy
	"KBB", // Bomberman Blitz
	"KAH", // Art Style: Boxlife
	"KKQ", // Bugs'N'Balls
	"K2J", // Cake Ninja
	"KCY", // Calculator
	"KC5", // Castle Conqueror: Heroes
	"KCV", // Cave Story
	"KUQ", // Chuck E. Cheese's Alien Defense Force
	"KUC", // Chuck E. Cheese's Arcade Room
	"KQL", // Chuukara! Dairoujou
	"KXF", // Color Commando
	"KDC", // Crash-Course Domo
	"K32", // CuteWitch! runner
	"KF3", // Dairojo! Samurai Defenders
	"KDV", // Dark Void Zero
	"KWT", // GO Series: Defense Wars
	"KHE", // DotMan
	"K9E", // Dreamwalker
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
	"K8M", // Model Academy
	"KXB", // Monster Buster Club
	"KMB", // Mr. Brain
	"KDR", // Mr. Driller: Drill Till You Drop
	"K2D", // Nintendo DSi + Internet
	"KSUE", // Number Battle
	"K6T", // Orion's Odyssey
	"KP9", // Paul's Monster Adventure
	"KPJ", // Paul's Shooting Adventure
	"KUS", // Paul's Shooting Adventure 2
	"KPQ", // GO Series: Picdun
	"KAP", // Art Style: PiCTOBiTS
	"KHR", // Picture Perfect: Pocket Stylist
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
	"KX5", // SnowBoard Xtreme
	"KA6", // Space Ace
	"K4D", // Sudoku
	"K4F", // Sudoku 4Pockets
	"K6P", // Unou to Sanougaren Sasuru: Uranoura
	"KVT", // VT Tennis
	"KK4", // Wakugumi: Monochrome Puzzle
	"Z2A", // WarioWare: Touched! DL
	"KDW", // White-Water Domo
	"KAS", // Art Style: ZENGAGE
};

// B4DS DSiWare Whitelist (Show RAM limitation message) (Total: 6)
static const char compatibleGameListB4DSRAMLimited[][4] = {
	"KAA", // Art Style: Aquia
	"KFP", // Flipper
	"KWY", // Mighty Milky Way
	"K6T", // Orion's Odyssey
	"KHR", // Picture Perfect: Pocket Stylist
	"KS3", // Shantae: Risky's Revenge
};

// B4DS DSiWare Whitelist (RAM limitation message ID)
static int compatibleGameListB4DSRAMLimitedID[] = {
	1, // Art Style: Aquia (No audio)
	2, // Flipper (No music)
	2, // Mighty Milky Way (No music)
	4, // Orion's Odyssey (Crashes at certain points)
	0, // Picture Perfect: Pocket Stylist (Parts of the game only)
	3, // Shantae: Risky's Revenge (Crashes at the Lighthouse after first fight)
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

// B4DS DSiWare Whitelist (DS Debug consoles with 8MB of RAM) (Total: 26)
static const char compatibleGameListB4DSDebug[][4] = {
	"KII", // 101 Pinball World
	"KXT", // 99Seconds
	"K27", // G.G. Series: All Breaker
	"KAB", // G.G. Series: Assault Buster
	"KBZ", // BlayzBloo: Super Melee Brawlers Battle Royale
	"K2N", // Cake Ninja 2
	"KYN", // Cake Ninja: XMAS
	"KCN", // Castle Conqueror
	"KQN", // Castle Conqueror: Against
	"KXC", // Castle Conqueror: Heroes 2
	"KQN", // Castle Conqueror: Revolution
	"KDQ", // Dragon Quest Wars
	"KFD", // Fieldrunners
	"KKN", // Flipper 2: Flush the Goldfish
	"K3G", // Go! Go! Kokopolo
	"KQ9", // The Legend of Zelda: Four Swords: Anniversary Edition
	"KYL", // Make Up & Style
	"K59", // Metal Torrent
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
