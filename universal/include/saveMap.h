#ifndef SAVEMAP_H
#define SAVEMAP_H

#include <map>
#include <set>

static const std::map<uint, std::set<std::string>> saveMap = { 
	{ 0, {		// None
		"Y5E", // Advance Wars - Days of Ruin (Kiosk Demo)
		"A43", // Big Brain Academy (Demo)
		"Y8G", // Dragon Quest IX - Sentinels of the Starry Skies (Video)
		"Y8A", // Dragon Quest Monsters - Joker 2 (Kiosk Demo)
		"A7A", // DS Download Station - Vol 1
		"A7B", // DS Download Station - Vol 2
		"A7C", // DS Download Station - Vol 3
		"A7D", // DS Download Station - Vol 4
		"A7E", // DS Download Station - Vol 5
		"A7F", // DS Download Station - Vol 6 (EUR)
		"A7G", // DS Download Station - Vol 6 (USA)
		"A7H", // DS Download Station - Vol 7
		"A7I", // DS Download Station - Vol 8
		"A7J", // DS Download Station - Vol 9
		"A7K", // DS Download Station - Vol 10
		"A7L", // DS Download Station - Vol 11
		"A7M", // DS Download Station - Vol 12
		"A7N", // DS Download Station - Vol 13
		"A7O", // DS Download Station - Vol 14
		"A7P", // DS Download Station - Vol 15
		"A7Q", // DS Download Station - Vol 16
		"A7R", // DS Download Station - Vol 17
		"A7S", // DS Download Station - Vol 18
		"A7T", // DS Download Station - Vol 19
		"Y5O", // Fossil Fighters (Kiosk Demo)
		"Y8L", // Golden Sun - Dark Dawn (Kiosk Demo)
		"Y2M", // Kirby - Squeak Squad (Demo)
		"Y52", // Kirby - Super Star Ultra (Demo)
		"Y4C", // The Legend of Zelda - Phantom Hourglass (Demo)
		"Y7S", // The Legend of Zelda - Spirit Tracks (Demo)
		"AM3", // Mario Kart DS (Kiosk Demo)
		"Y4Y", // Mario Party DS (Demo)
		"V9A", // Mario vs. Donkey Kong - Mini-Land Mayhem! (Kiosk Demo)
		"A76", // Metroid Prime - Hunters (Demo)
		"A85", // New Super Mario Bros. (Demo)
		"Y5J", // Ninja Gaiden - Dragon Sword (Kiosk Demo)
		"Y6F", // Personal Trainer - Cooking (Kiosk Demo)
		"A79", // Pokemon Ranger (Demo)
		"Y49", // Professor Layton and the Curious Village (Demo) 
		"Y6Z", // Professor Layton and the Diabolical Box (Demo) 
		"Y8I", // Professor Layton and the Unwound Future (Demo Video)
		"Y9B", // Professor Layton and the Last Specter (Demo)
		"Y7G", // Style Savvy (Demo)
		"A72", // Tetris DS (Kiosk Demo)
		"Y2J", // Wario - Master of Disguise (Demo)
	}},
	{ 0x200, {		// 1 KB/4 kbit
		"CLF", // Code Lyoko - Fall of X.A.N.A.
		"YGO", // Dragon Hunters
		"B2J", // Dragon Quest Monsters - Joker 2 Professional
		"YEN", // ElvenLand
		"YFR", // Ferrari Challenge - Trofeo Pirelli
		"CGF", // Garfield's Fun Fest
		"YGJ", // George of the Jungle
		"CHK", // Hell's Kitchen - The Game
		"AL4", // Illumislight - Hikari no Puzzle
		"CIT", // Imagine - Teacher
		"YJB", // LEGO Batman - The Videogame
		"YEL", // Level5 Premium Silver (Demo)
		"YEW", // Level5 Premium Gold (Demo)
		"B5P", // Level5 Premium Platinum (Demo)
		"CIL", // Long Vacation - Iruka to Watashi
		"CNL", // Miteha Ikenai
		"CRP", // The Price is Right
		"YFW", // Rain Drops
		"CGX", // Shinreigari - Ghost Hound DS
		"YZL", // Simple DS Series Vol. 44 - The Gal Mahjong
	}},
	{ 0x2000, {		// 8 KB/64 kbit
		"YBD", // Bikkuriman Daijiten
		"YTJ", // The Cheetah Girls - Passport to Stardom
		"YCE", // Chessmaster - The Art of Learning
		"CSH", // Chi's Sweet Home - Chi ga Ouchi ni Yattekita!
		"YJV", // Clever! Das Spiel, das Wissen Schafft
		"CD2", // Code Geass - Hangyaku no Lelouch R2 - Banjou no Geass Gekijou
		"CG7", // Dragon Ball - Origins
		"BDB", // Dragon Ball - Origins 2
		"ADB", // Dragon Ball Z - Supersonic Warriors 2
		"ADQ", // Dragon Quest Heroes - Rocket Slime
		"CJR", // Dragon Quest Monsters - Joker 2
		"CDP", // DropCast
		"AER", // Ecolis
		"YM8", // Gakken Mu Henshuubu Kanshuu - Choujou Genshou Research File
		"CAI", // Gyakkyou Burai Kaiji - Death or Survival
		"CMA", // Hoshizora no Comic Garden
		"YX2", // Kageyama Method - Masu x Masu Pure Hyaku Masu Keisan - Hyaku Masu no Maeni Kore Dayo!
		"AKW", // Kirby - Squeak Squad
		"CKN", // Knights in the Nightmare
		"CLH", // Little League World Series Baseball 2008
		"A8T", // Mario Party DS
		"AMK", // Mezase! Koushien
		"AVL", // Mizuiro Blood
		"CNY", // My Chinese Coach
		"YFC", // Mystery Case Files - MillionHeir
		"A2D", // New Super Mario Bros.
		"YNI", // Ninja Reflex
		"YU6", // Panda-San Nikki
		"YRI", // Playmobil Interactive - Pirates Boarding
		"CP4", // Princess Maker 4 DS - Special Edition
		"CPC", // Puzzler Collection
		"YG6", // Race Driver - GRID
		"CM6", // Rhapsody - A Musical Adventure
		"YLZ", // Rhythm Heaven
		"CX3", // San-X Character Channel - All-Star Daishuugou!
		"A3M", // Shin Sangoku Musou DS - Fighter's Battle
		"ASC", // Sonic Rush
		"ASM", // Super Mario 64 DS
		"CQE", // The Quest Trio - Jewels, Cards and Tiles
		"YGC", // Quick Yoga Training - Learn in Minutes a Day
		"ATR", // Tetris DS
		"YJA", // Theresia.. - Dear Emile
		"YUP", // U-Can - Penji Training DS
		"CUU", // Umiuru to Sudoku Shiyo! - Nikoli Gensen 7 Dai Puzzle 555 Mon
		"AWA", // Wario - Master of Disguise
		"CZK", // Zettai Karen Children DS - Dai-4 no Children
	}},
	{ 0x10000, {		// 64 KB/512 kbit
		"YQU", // Chrono Trigger
		"CTL", // City Life DS
		"YDI", // Digimon World Championship
		"YIV", // Dragon Quest IV - Chapters of the Chosen
		"YV5", // Dragon Quest V - Hand of the Heavenly Bride
		"YVI", // Dragon Quest VI - Realms of Revelation
		"YDQ", // Dragon Quest IX - Sentinels of the Starry Skies
		"AXJ", // Dungeon Explorer
		"YVS", // From the Abyss
		"CHU", // Harukanaru Toki no Naka de - Yume no Ukihashi
		"YEE", // Inazuma Eleven
		"BEE", // Inazuma Eleven 2 - Firestorm
		"BEB", // Inazuma Eleven 2 - Blizzard
		"BEZ", // Inazuma Eleven 3 - Bomb Blast
		"BE8", // Inazuma Eleven 3 - Lightning Bolt
		"BOE", // Inazuma Eleven 3 - Team Ogre Attacks!
		"C6C", // Infinite Space
		"YKW", // Kirby - Super Star Ultra
		"CKU", // Kumatanchi
		"CLR", // Line Rider 2 - Unbound
		"CSP", // Lock's Quest
		"CMY", // Matching Maker DS
		"CWM", // Moe Sta - Moeru Toudai Eigojuku
		"YE6", // Moetan DS
		"AFE", // New International Hyper Sports DS
		"YNG", // Ninja Gaiden - Dragon Sword
		"CEB", // Okada Toshio no Itsumademo DEBU to Omounayo - DS Recording Diet
		"YZK", // Quiz Magic Academy DS
		"CGM", // Sigma Harmonics
		"CAP", // The Sims 2 - Apartment Pets
		"A3Y", // Sonic Rush Adventure
		"YHL", // Time Hollow
		"YYM", // Tokyo Majin Gakuen - Kenpuuchou
		"CUW", // Uwasa no Midori-kun!! 2 - Futari no Midori!
	}},
	{ 0x40000, {	// 256 KB / 2 Mbit
		"YBN", // 100 Classic Book Collection
		"YJZ", // 1 Nichi 10 Pun de E ga Jouzu ni Kakeru DS
		"UNS", // 1Seg Jushin Adaptor - DS TV
		"YW2", // Advance Wars - Days of Ruin
		"AWR", // Advance Wars - Dual Strike
		"ADM", // Animal Crossing - Wild World
		"CAN", // Anno 1701 - Dawn of Discovery
		"A6J", // Arasuji de Kitaeru Hayamimi no Susume DS
		"BAR", // A Ressha de Ikou DS
		"YAS", // Ash - Archaic Sealed Heat
		"APX", // Atsumare! Power Pro Kun no DS Koushien
		"AY6", // Bangai-O Spirits
		"AZ6", // Beetle King
		"YYB", // Blue Dragon Plus
		"YB3", // Bokujou Monogatari - KiraKira Taiyou to Nakamatachi
		"CB9", // Bokujou Monogatari - Youkoso! Kaze no Bazaar e
		"AND", // Brain Age - Train Your Brain in Minutes a Day!
		"ANM", // Brain Age 2 - More Training in Minutes a Day!
		"CBG", // Bratz - Girlz Really Rock
		"AJB", // Chibi-Robo! - Park Patrol
		"ACO", // Contact
		"COV", // The Conveni DS - Otona no Keieiryoku Training
		"YPD", // Cosmetic Paradise
		"YCJ", // Crazy Frog Collectables - Art School
		"ACK", // Croket! DS - Tenkuu no Yuushatachi
		"YKR", // Culdcept DS
		"ABB", // Daigasso! Band Brothers
		"A8R", // Daito Giken Koushiki Pachi-Slot Simulator Hihouden - Ossu Banchou - Yoshimune DS
		"AUM", // Derby Stallion DS
		"AWD", // Diddy Kong Racing DS
		"ARL", // Doko Demo Raku Raku! DS Kakeibo
		"ABU", // Donkey Kong - Jungle Climber
		"AJR", // Dragon Quest Monsters - Joker
		"AGO", // Dragon Tamer - Sound Spirits
		"YDW", // Drawn to Life
		"BDR", // Drawn to Life - The Next Chapter
		"CDL", // Drawn to Life - SpongeBob SquarePants Edition
		"CCE", // Drivers Ed Portable
		"YBN", // DS Bungaku Zenshuu
		"ADJ", // DS Rakubiki Jiten
		"YVK", // DS Vitamin - Health Food Guide!
		"YDH", // Ducati Moto
		"ANG", // Eigo ga Nigate na Otona no DS Training - Eigo Zuke
		"AOS", // Elite Beat Agents
		"CPX", // Emily/Pocketbook - My Personal Diary
		"ANG", // English Training - Have Fun Improving Your Skills
		"YIK", // Etrian Odyssey II - Heroes of Lagaard
		"THM", // FabStyle
		"YKO", // Facening de Hyoujou Yutaka ni Inshou Up - Otana no DS Kao Training
		"YFE", // Fire Emblem - Shadow Dragon
		"AG3", // Flash Focus - Vision Training in Minutes a Day
		"CFF", // Football Director DS
		"YKH", // Fossil Fighters
		"ACH", // Freshly Picked - Tingle's Rosy Rupeeland
		"BO5", // Golden Sun - Dark Dawn
		"AGR", // Guru Guru
		"ABC", // Harvest Moon DS
		"AB4", // Harvest Moon DS Cute
		"ABJ", // Harvest Moon DS - Island of Happiness
		"YSR", // Hello Kitty - Daily
		"YEK", // Hercules no Eikou - Tamashii no Shoumei
		"AHE", // Heroes of Mana
		"AWI", // Hotel Dusk - Room 215
		"A2H", // I Did It Mum! - Boy
		"A2I", // I Did It Mum! - Girl
		"BCN", // Irodzuki Tingle no Koi no Balloon Trip
		"BIM", // Iron Master - The Legendary Blacksmith
		"AWG", // Jet Impulse
		"AJG", // Jinsei Game DS
		"ALA", // Jissen Pachi-Slot Hisshouhou! DS - Aladdin 2 Evolution
		"APS", // Jissen Pachi-Slot Hisshouhou! Hokuto no Ken DS
		"APJ", // Jissen Pachi-Slot Hisshouhou! Hokuto no Ken SE DS
		"YKI", // K-1 World GP - Zettai Ouja Ikusei Keikaku
		"AOI", // Kenkou Ouen Recipe 1000 - DS Kondate Zenshuu
		"AK7", // Kero Kero 7
		"YKD", // Ketsui - Death Label
		"AKL", // Kirarin Revolution - Kira Kira Idol Audition
		"AR2", // Kirarin ' Revolution - Naasan to Issho
		"TAD", // Kirby - Mass Attack
		"AIM", // Kokoro ni Shimiru - Mouhitsu de Kaku - Aida Mitsuo DS
		"YJ9", // Korg DS-10 Synthesizer
		"YCH", // Kousoku Card Battle - Card Hero
		"A2K", // Kurikin - Nano Island Story
		"AK2", // LifeSigns - Surgical Unit
		"ANI", // Luminous Arc
		"AMD", // Madden NFL 2005
		"AM6", // Madden NFL 06
		"ANF", // Madden NFL 07
		"A5U", // Madden NFL 08
		"CMD", // Madden NFL 09
		"AVC", // Magical Starsign
		"AC5", // Mahjong Fight Club DS - Wi-Fi Taiou
		"YKM", // Mainichi ga Tanoshii! - Ayanokouji Kimimaro no Happy Techou
		"AIO", // Make 10 - A Journey of Numbers
		"AM5", // Mar Heaven - Boukyaku no Clavia
		"AM2", // Mar Heaven - Karudea no Akuma
		"AMC", // Mario Kart DS
		"AMQ", // Mario vs. Donkey Kong - March of the Minis
		"YLD", // Master of the Monster Lair
		"A5T", // MegaMan Battle Network 5 - Double Team DS
		"A6A", // MegaMan Star Force - Pegasus
		"A6B", // MegaMan Star Force - Leo
		"A6C", // MegaMan Star Force - Dragon
		"YRV", // MegaMan Star Force 2 - Zerker x Ninja
		"YRW", // MegaMan Star Force 2 - Zerker x Saurian
		"CRB", // MegaMan Star Force 3 - Black Ace
		"CRR", // MegaMan Star Force 3 - Red Joker
		"AMH", // Metroid Prime - Hunters
		"ACY", // Mezase! Shoujo Manga Ka! Chao Manga School
		"AG3", // Miru Chikara wo Jissen de Kitaeru - DS Medikara Training
		"CPP", // MLB Power Pros 2008
		"AO2", // Moero! Nekketsu Rhythm Damashii - Osu! Tatakae! Ouendan 2
		"YMS", // Mushishi - Ame Furu Sato
		"CKZ", // My DoItAll
		"AQI", // MySims
		"C38", // MySims Agents
		"CK5", // MySims Kingdom
		"AN2", // Naruto RPG 2 - Chidori vs Rasengan
		"BET", // Nihon Keizai Shinbunsha Kanshuu - Mono ya Okane no Shikumi DS
		"UBR", // Nintendo DS Browser
		"AD5", // Nintendogs - Best Friends
		"AD2", // Nintendogs - Chihuahua & Friends
		"ADG", // Nintendogs - Dachshund & Friends
		"AD7", // Nintendogs - Dalmatian & Friends
		"AD3", // Nintendogs - Lab & Friends
		"AGF", // Nintendo Touch Golf - Birdie Challenge
		"ANB", // Nobunaga no Yabou DS
		"CN2", // Nobunaga no Yabou DS 2
		"YIQ", // Nounai Aesthe - IQ Suppli DS 2 - Sukkiri King Ketteisen
		"B62", // Okaeri! Chibi-Robo! Happy Rich Oosouji
		"YOE", // Orcs & Elves
		"AJY", // Otona no Joushikiryoku Training DS
		"AON", // Paint by DS
		"YU2", // Paint by DS - Classic Masterpieces
		"AO5", // Paint by DS - Military Vehicles (Zen Series)
		"CNV", // Personal Trainer - Cooking
		"AGY", // Phoenix Wright - Ace Attorney
		"A8N", // Planet Puzzle League
		"APH", // Pokemon Mystery Dungeon - Blue Rescue Team
		"YFY", // Pokemon Mystery Dungeon - Explorers of Darkness
		"C2S", // Pokemon Mystery Dungeon - Explorers of Sky
		"YFT", // Pokemon Mystery Dungeon - Explorers of Time
		"ARG", // Pokemon Ranger
		"YP2", // Pokemon Ranger - Shadows of Almia
		"APK", // Power Pocket Koushien
		"AP8", // Power Pro Kun Pocket 8
		"API", // Power Pro Kun Pocket 9
		"YPJ", // Power Pro Kun Pocket 10
		"CXI", // Power Pro Kun Pocket 11
		"VPT", // Power Pro Kun Pocket 12
		"VPL", // Power Pro Kun Pocket 13
		"YW8", // Pro Evolution Soccer 2008
		"ADH", // Project Hacker - Kakusei
		"C29", // Pro Yakyuu Famista DS 2009
		"B89", // Pro Yakyuu Team o Tsukurou! 2
		"YPT", // Puppy Palace
		"AJZ", // Puzzle Series Vol. 13 - Kanji Puzzle
		"BPV", // Puzzler World
		"C48", // Puzzling - Masterpiece Exploration DS
		"AKA", // The Rub Rabbits!
		"A3G", // San Goku Shi DS
		"A3F", // San Goku Shi DS 2
		"ASG", // SD Gundam G Generation DS
		"AUZ", // The Settlers
		"A4V", // Shaberu! DS Oryouri Navi
		"CBO", // Shibou Nenshou Keikaku - YaseTore!! DS
		"YVU", // Shiseido Beauty Solution Kaihatsu Center Kanshuu - Project Beauty
		"YS6", // Sid Meier's Civilization Revolution
		"AG3", // Sight Training
		"AC3", // SimCity DS
		"YC2", // SimCity DS - Creator
		"YZQ", // Simple DS Series Vol. 26 - The Quiz 30000-Mon
		"CSV", // Skate It
		"YSP", // Spore Creatures
		"C49", // Spore Hero Arena
		"ASJ", // The Sims 2
		"YS2", // The Sims 2 - Castaway
		"A4O", // The Sims 2 - Pets
		"YID", // Smart Kid's Gameclub
		"CSZ", // Smart Kid's Party Fun Pak
		"ASF", // Star Fox Command
		"ABS", // Style Book - Cinnamoroll
		"ABF", // Style Book - Fushigi Boshi no Futago Hime
		"ABI", // Style Book - Junior City
		"AS6", // Super Robot Taisen W
		"YSD", // Super Robot Taisen K
		"AUB", // Tabi no Yubisashi Kaiwachou DS - DS Series 1 Thai
		"AUC", // Tabi no Yubisashi Kaiwachou DS - DS Series 2 China
		"AUE", // Tabi no Yubisashi Kaiwachou DS - DS Series 3 Korea
		"AUA", // Tabi no Yubisashi Kaiwachou DS - DS Series 4 America
		"AUD", // Tabi no Yubisashi Kaiwachou DS - DS Series 5 Germany
		"ALD", // Tao's Adventure - Curse of the Demon Seal
		"CTB", // Tecmo Bowl - Kickoff
		"ACU", // Tenchu Dark Secret
		"A8Q", // Theme Park
		"CX4", // Tokutenryoku Gakushuu DS - Chuugaku Jitsugi 4 Kyouka
		"YXK", // Tokutenryoku Gakushuu DS - Chuu 2 Eisuukoku Pack
		"ATC", // Tom Clancy's Splinter Cell - Chaos Theory
		"YUD", // Tomodachi Tsukurou! Mahou no Koukan Nikki
		"AH9", // Tony Hawk's American Sk8land
		"AWK", // Tony Hawk's Downhill Jam
		"ATP", // Touch de Rakushou! Pachislo Sengen - Rio de Carnival
		"AIX", // Touch de Tanoshimu Hyakunin Isshu - DS Shigureden
		"C5T", // Treasure World
		"CP3", // Viva Pinata - Pocket Paradise
		"AYK", // Wi-Fi Taiou - Yakuman DS
		"AWE", // Winning Eleven Pro Evolution Soccer 2007
		"YGD", // Winx Club - Secret Diary 2009
		"YSL", // World Destruction - Michibikareshi Ishi
		"AYJ", // Yakitate!! Japan Game 1 Gou Choujou Kessen!! Pantasic Grand Prix!
		"AMJ", // Yakuman DS
		"AYX", // Yu-Gi-Oh! GX - Spirit Caller
		"AY7", // Yu-Gi-Oh! World Championship 2007
		"YG8", // Yu-Gi-Oh! World Championship 2008
		"AZZ", // Zettai Zetsumei Dangerous Jiisan DS - Dangerous Sensation
		"AZX", // Zoids Battle Colosseum
		"AZS", // Zoids Saga DS - Legend of Arcadia
		"YZT", // Zoo Tycoon 2 DS
	}},
	{ 0x80000, {	// 512 KB / 4 Mbit
		"VET", // 1000 Cooking Recipes from Elle a Table
		"BKC", // America's Test Kitchen - Let's Get Cooking
		"YC3", // Chungjeon! Hanguginui Sangsingnyeok DS
		"A2Y", // Ganbaru Watashi no Kakei Diary
		"CDG", // Disgaea DS
		"AVM", // DS Bimoji Training
		"CO4", // Karada Support Kenkyuujo - Tounyoubyou Hen
		"YQJ", // MagicQ DS
		"V2G", // Mario vs. Donkey Kong - Mini-Land Mayhem
		"AZE", // The Legend of Zelda - Phantom Hourglass
		"IMW", // Personal Trainer - Walking
		"ADA", // Pokemon - Diamond Version
		"APA", // Pokemon - Pearl Version
		"CPU", // Pokemon - Platinum Version
		"IPK", // Pokemon - HeartGold Version
		"IPG", // Pokemon - SoulSilver Version
		"VP4", // Power Pro Kun Pocket 14
		"ANH", // Practice English
		"CYA", // Taiheiyou no Arashi DS - Senkan Yamato, Akatsuki ni Shutsugeki Su!
		"CCU", // Tomodachi Collection
		"A3A", // Wi-Fi Baken Yosouryoku Training - Umania - 2007 Nendo Ban
	}},
	{ 0x100000, {	// 1 MB / 8 Mbit
		"BKI", // The Legend of Zelda - Spirit Tracks
		"C6P", // Picross 3D
		"AZL", // Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
	}},
	{ 0x800000, {	// 8 MB
		"VAA", // Art Academy
		"UXB", // Jam with the Band
	}},
	{ 0x2000000, {	// 32 MB
		"UOR", // WarioWare - D.I.Y. (Do It Yourself)
	}}
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
