#ifndef SAVEMAP_H
#define SAVEMAP_H

#include <map>
#include <set>

static const std::map<uint, std::set<std::string>> saveMap = { 
	{ 0x2000, {		// 8KB
		"ASC", // Sonic Rush
	}},
	{ 0x40000, {	// 256KB
		"YJZ", // 1 Nichi 10 Pun de E ga Jouzu ni Kakeru DS
		"UNS", // 1Seg Jushin Adaptor - DS TV
		"YW2", // Advance Wars - Days of Ruin
		"AWR", // Advance Wars - Dual Strike
		"ADM", // Animal Crossing - Wild World
		"A6J", // Arasuji de Kitaeru Hayamimi no Susume DS
		"YAS", // Ash - Archaic Sealed Heat
		"APX", // Atsumare! Power Pro Kun no DS Koushien
		"AY6", // Bangai-O Spirits
		"AZ6", // Beetle King
		"YYB", // Blue Dragon Plus
		"YB3", // Bokujou Monogatari - KiraKira Taiyou to Nakamatachi
		"YKH", // Bokura wa Kaseki Horider
		"AND", // Brain Age - Train Your Brain in Minutes a Day!
		"ANM", // Brain Age 2 - More Training in Minutes a Day!
		"CBG", // Bratz - Girlz Really Rock
		"AJB", // Chibi-Robo! - Park Patrol
		"ACO", // Contact
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
		"CDL", // Drawn to Life - SpongeBob SquarePants Edition
		"YBN", // DS Bungaku Zenshuu
		"ADJ", // DS Rakubiki Jiten
		"YVK", // DS Vitamin - Health Food Guide!
		"YDH", // Ducati Moto
		"YLD", // Dungeon Maker - Mahou no Shovel to Chiisana Yuusha
		"ANG", // Eigo ga Nigate na Otona no DS Training - Eigo Zuke
		"AOS", // Elite Beat Agents
		"ANG", // English Training - Have Fun Improving Your Skills
		"YIK", // Etrian Odyssey II - Heroes of Lagaard
		"THM", // FabStyle
		"YKO", // Facening de Hyoujou Yutaka ni Inshou Up - Otana no DS Kao Training
		"YFE", // Fire Emblem - Shadow Dragon
		"AG3", // Flash Focus - Vision Training in Minutes a Day
		"ACH", // Freshly Picked - Tingle's Rosy Rupeeland
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
		"AIM", // Kokoro ni Shimiru - Mouhitsu de Kaku - Aida Mitsuo DS
		"YU2", // Kokoro o Yasumeru - Otona no Nurie DS 2
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
		"AMH", // Metroid Prime Hunters
		"ACY", // Mezase! Shoujo Manga Ka! Chao Manga School
		"AG3", // Miru Chikara wo Jissen de Kitaeru - DS Medikara Training
		"CPP", // MLB Power Pros 2008
		"AO2", // Moero! Nekketsu Rhythm Damashii - Osu! Tatakae! Ouendan 2
		"YMS", // Mushishi - Ame Furu Sato
		"AQI", // MySims
		"CK5", // MySims Kingdom
		"AN2", // Naruto RPG 2 - Chidori vs Rasengan
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
		"YOE", // Orcs & Elves
		"AJY", // Otona no Joushikiryoku Training DS
		"AON", // Paint by DS
		"AO5", // Paint by DS - Military Vehicles (Zen Series)
		"CNV", // Personal Trainer - Cooking
		"AGY", // Phoenix Wright - Ace Attorney
		"A8N", // Planet Puzzle League
		"APH", // Pokemon Mystery Dungeon - Blue Rescue Team
		"YFY", // Pokemon Mystery Dungeon - Explorers of Darkness
		"YFT", // Pokemon Mystery Dungeon - Explorers of Time
		"ARG", // Pokemon Ranger
		"YP2", // Pokemon Ranger - Shadows of Almia
		"APK", // Power Pocket Koushien
		"AP8", // Power Pro Kun Pocket 8
		"API", // Power Pro Kun Pocket 9
		"YPJ", // Power Pro Kun Pocket 10
		"CXI", // Power Pro Kun Pocket 11
		"YW8", // Pro Evolution Soccer 2008
		"ADH", // Project Hacker - Kakusei
		"YPT", // Puppy Palace
		"AJZ", // Puzzle Series Vol. 13 - Kanji Puzzle
		"AKA", // The Rub Rabbits!
		"A3G", // San Goku Shi DS
		"A3F", // San Goku Shi DS 2
		"ASG", // SD Gundam G Generation DS
		"AUZ", // The Settlers
		"A4V", // Shaberu! DS Oryouri Navi
		"YS6", // Sid Meier's Civilization Revolution
		"AG3", // Sight Training
		"AC3", // SimCity DS
		"YC2", // SimCity DS - Creator
		"YZQ", // Simple DS Series Vol. 26 - The Quiz 30000-Mon
		"CSV", // Skate It
		"YSP", // Spore Creatures
		"ASJ", // The Sims 2
		"YS2", // The Sims 2 - Castaway
		"A4O", // The Sims 2 - Pets
		"YID", // Smart Kid's Gameclub
		"ASF", // Star Fox Command
		"ABS", // Style Book - Cinnamoroll
		"ABF", // Style Book - Fushigi Boshi no Futago Hime
		"ABI", // Style Book - Junior City
		"AS6", // Super Robot Taisen W
		"AUB", // Tabi no Yubisashi Kaiwachou DS - DS Series 1 Thai
		"AUC", // Tabi no Yubisashi Kaiwachou DS - DS Series 2 China
		"AUE", // Tabi no Yubisashi Kaiwachou DS - DS Series 3 Korea
		"AUA", // Tabi no Yubisashi Kaiwachou DS - DS Series 4 America
		"AUD", // Tabi no Yubisashi Kaiwachou DS - DS Series 5 Germany
		"ALD", // Tao's Adventure - Curse of the Demon Seal
		"CTB", // Tecmo Bowl - Kickoff
		"ACU", // Tenchu Dark Secret
		"A8Q", // Theme Park
		"ATC", // Tom Clancy's Splinter Cell - Chaos Theory
		"AH9", // Tony Hawk's American Sk8land
		"AWK", // Tony Hawk's Downhill Jam
		"ATP", // Touch de Rakushou! Pachislo Sengen - Rio de Carnival
		"AIX", // Touch de Tanoshimu Hyakunin Isshu - DS Shigureden
		"CP3", // Viva Pinata - Pocket Paradise
		"AYK", // Wi-Fi Taiou - Yakuman DS
		"AWE", // Winning Eleven Pro Evolution Soccer 2007
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
	{ 0x80000, {	// 512KB
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
		"IPK", // Pokemon - HeartGold Version
		"IPG", // Pokemon - SoulSilver Version
		"VP4", // Power Pro Kun Pocket 14
		"ANH", // Practice English
		"CYA", // Taiheiyou no Arashi DS - Senkan Yamato, Akatsuki ni Shutsugeki Su!
		"A3A", // Wi-Fi Baken Yosouryoku Training - Umania - 2007 Nendo Ban
	}},
	{ 0x100000, {	// 1MB
		"BKI", // The Legend of Zelda - Spirit Tracks
		"C6P", // Picross 3D
		"AZL", // Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
	}},
	{ 0x800000, {	// 8MB
		"VAA", // Art Academy
		"UXB", // Jam with the Band
	}},
	{ 0x2000000, {	// 32MB
		"UOR", // WarioWare - D.I.Y. (Do It Yourself)
	}}
};

#endif // SAVEMAP_H
