#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/nitrofs.h"
#include "common/systemdetails.h"
#include "common/logging.h"
#include "myDSiMode.h"
#include "perGameSettings.h"
#include "graphics/graphics.h"
#include <gl2d.h>

#include "ndsheaderbanner.h"
#include "module_params.h"

static u32 arm9Sig[3][4];

extern sNDSBannerExt ndsBanner;

bool checkDsiBinaries(FILE* ndsFile) {
	sNDSHeaderExt ndsHeader;

	fseek(ndsFile, 0, SEEK_SET);
	displayDiskIcon(ms().secondaryDevice);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);

	if (ndsHeader.unitCode == 0) {
		displayDiskIcon(false);
		return true;
	}

	if (ndsHeader.arm9iromOffset < 0x8000 || ndsHeader.arm9iromOffset >= 0x20000000
	 || ndsHeader.arm7iromOffset < 0x8000 || ndsHeader.arm7iromOffset >= 0x20000000) {
		displayDiskIcon(false);
		return false;
	}

	for (int i = 0; i < 3; i++) {
		arm9Sig[i][0] = 0;
		arm9Sig[i][1] = 0;
		arm9Sig[i][2] = 0;
		arm9Sig[i][3] = 0;
	}

	fseek(ndsFile, 0x8000, SEEK_SET);
	fread(arm9Sig[0], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm9iromOffset, SEEK_SET);
	fread(arm9Sig[1], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm7iromOffset, SEEK_SET);
	fread(arm9Sig[2], sizeof(u32), 4, ndsFile);
	displayDiskIcon(false);
	for (int i = 1; i < 3; i++) {
		if (arm9Sig[i][0] == arm9Sig[0][0]
		 && arm9Sig[i][1] == arm9Sig[0][1]
		 && arm9Sig[i][2] == arm9Sig[0][2]
		 && arm9Sig[i][3] == arm9Sig[0][3]) {
			return false;
		}
		if (arm9Sig[i][0] == 0
		 && arm9Sig[i][1] == 0
		 && arm9Sig[i][2] == 0
		 && arm9Sig[i][3] == 0) {
			return false;
		}
		if (arm9Sig[i][0] == 0xFFFFFFFF
		 && arm9Sig[i][1] == 0xFFFFFFFF
		 && arm9Sig[i][2] == 0xFFFFFFFF
		 && arm9Sig[i][3] == 0xFFFFFFFF) {
			return false;
		}
	}

	return true;
}

/**
 * Get the title ID.
 * @param ndsFile DS ROM image.
 * @param buf Output buffer for title ID. (Must be at least 4 characters.)
 * @return 0 on success; non-zero on error.
 */
int grabTID(FILE *ndsFile, char *buf)
{
	fseek(ndsFile, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	size_t read = fread(buf, 1, 4, ndsFile);
	return !(read == 4);
}

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
u32 getSDKVersion(FILE *ndsFile)
{
	sNDSHeaderExt NDSHeader;
	fseek(ndsFile, 0, SEEK_SET);
	fread(&NDSHeader, 1, sizeof(NDSHeader), ndsFile);
	if (NDSHeader.arm7destination >= 0x037F8000)
		return 0;
	return getModuleParams(&NDSHeader, ndsFile)->sdk_version;
}

/**
 * Check if NDS game has AP.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 1 or 2 on success; 0 if no AP.
 */
int checkRomAP(FILE *ndsFile, const char* filename)
{
	displayDiskIcon(!sys().isRunFromSD());

	char game_TID[5];
	u16 headerCRC16 = 0;
	fseek(ndsFile, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, ndsFile);
	fseek(ndsFile, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
	fread(&headerCRC16, sizeof(u16), 1, ndsFile);
	game_TID[4] = 0;

	{
		char apFixPath[256];
		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s.ips", sys().isRunFromSD() ? "sd" : "fat", filename);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			displayDiskIcon(false);
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s.bin", sys().isRunFromSD() ? "sd" : "fat", filename);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			displayDiskIcon(false);
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s-%04X.ips", sys().isRunFromSD() ? "sd" : "fat", game_TID, headerCRC16);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			displayDiskIcon(false);
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s-%04X.bin", sys().isRunFromSD() ? "sd" : "fat", game_TID, headerCRC16);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			displayDiskIcon(false);
			return 0;
		}
	}

	{
		const bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
		char bootstrapPath[256];
		sprintf(bootstrapPath, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "sd" : "fat", useNightly ? "nightly" : "release");
		if (access(bootstrapPath, F_OK) != 0) {
			sprintf(bootstrapPath, "%s:/_nds/nds-bootstrap-%s.nds", sys().isRunFromSD() ? "fat" : "sd", useNightly ? "nightly" : "release");
		}

		bootFSInit(bootstrapPath);
	}

	FILE *file = fopen("boot:/apfix.pck", "rb");
	if (file) {
		char buf[5] = {0};
		fread(buf, 1, 4, file);
		if (strcmp(buf, ".PCK") == 0) { // Make sure correct file type
			u32 fileCount;
			fread(&fileCount, 1, sizeof(fileCount), file);
			logPrint("Searching for AP-fix...\n");

			// Try binary search for the game
			int left = 0;
			int right = fileCount;
			bool tidFound = false;

			while (left <= right) {
				fseek(file, 16 + left * 16, SEEK_SET);
				fread(buf, 1, 4, file);
				int cmp = strcmp(buf, game_TID);
				if (cmp == 0) { // TID matches, check CRC
					tidFound = true;
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);
					logPrint("TID match: %s, CRC: %04X\n", game_TID, crc);

					if (crc == 0xFFFF || crc == headerCRC16) { // CRC matches
						fclose(file);
						logPrint("AP-fix found!\n");
						displayDiskIcon(false);
						return 0;
					} else {
						left++;
					}
				} else if (tidFound) {
					break;
				} else {
					left++;
				}
			}
		}

		fclose(file);
	}
	logPrint("AP-fix not found!\n");
	displayDiskIcon(false);

	// Check for SDK4-5 ROMs that don't have AP measures.
	if ((memcmp(game_TID, "AZLJ", 4) == 0)   	// Girls Mode (JAP version of Style Savvy)
	 || (memcmp(game_TID, "YEEJ", 4) == 0)   	// Inazuma Eleven (Japan)
	 || (memcmp(game_TID, "CNSX", 4) == 0)   	// Naruto Shippuden: Naruto vs Sasuke (Europe)
	 || (memcmp(game_TID, "BH2J", 4) == 0)) {	// Super Scribblenauts (Japan)
		return 0;
	} else
	// Check for ROMs that have AP measures.
	if ((memcmp(game_TID, "VETP", 4) == 0)   	// 1000 Cooking Recipes from Elle a Table (Europe)
	 || (memcmp(game_TID, "CQQP", 4) == 0)   	// AFL Mascot Manor (Australia)
	 || (memcmp(game_TID, "CA5E", 4) == 0)   	// Again: Interactive Crime Novel (USA)
	 || (memcmp(game_TID, "TAKJ", 4) == 0)   	// All Kamen Rider: Rider Generation (Japan)
	 || (memcmp(game_TID, "BKCE", 4) == 0)   	// America's Test Kitchen: Let's Get Cooking (USA)
	 || (memcmp(game_TID, "A3PJ", 4) == 0)   	// Anpanman to Touch de Waku Waku Training (Japan)
	 || (memcmp(game_TID, "B2AK", 4) == 0)   	// Aranuri: Badachingudeulkkwa hamkke Mandeuneun Sesang (Korea)
	 || (memcmp(game_TID, "BB4J", 4) == 0)   	// Battle Spirits Digital Starter (Japan)
	 || (memcmp(game_TID, "CYJJ", 4) == 0)   	// Blood of Bahamut (Japan)
	 || (memcmp(game_TID, "TBSJ", 4) == 0)   	// Byoutai Seiri DS: Image Dekiru! Shikkan, Shoujou to Care (Japan)
	 || (memcmp(game_TID, "C5YJ", 4) == 0)   	// Chocobo to Mahou no Ehon: Majo to Shoujo to 5-nin no Yuusha (Japan)
	 || (memcmp(game_TID, "C6HK", 4) == 0)   	// Chuldong! Rescue Force DS (Korea)
	 || (memcmp(game_TID, "CCTJ", 4) == 0)   	// Cid to Chocobo no Fushigi na Dungeon: Toki Wasure no Meikyuu DS+ (Japan)
	 || (memcmp(game_TID, "CLPD", 4) == 0)   	// Club Penguin: Elite Penguin Force (Germany)
	 || (memcmp(game_TID, "BQ6J", 4) == 0)   	// Cocoro no Cocoron (Japan)
	 || (memcmp(game_TID, "BQIJ", 4) == 0)   	// Cookin' Idol I! My! Mine!: Game de Hirameki! Kirameki! Cooking (Japan)
	 || (memcmp(game_TID, "B3CJ", 4) == 0)   	// Cooking Mama 3 (Japan)
	 || (memcmp(game_TID, "TMCP", 4) == 0)   	// Cooking Mama World: Combo Pack: Volume 1 (Europe)
	 || (memcmp(game_TID, "TMDP", 4) == 0)   	// Cooking Mama World: Combo Pack: Volume 2 (Europe)
	 || (memcmp(game_TID, "BJ8P", 4) == 0)   	// Cooking Mama World: Hobbies & Fun (Europe)
	 || (memcmp(game_TID, "VCPJ", 4) == 0)   	// Cosmetick Paradise: Kirei no Mahou (Japan)
	 || (memcmp(game_TID, "VCTJ", 4) == 0)   	// Cosmetick Paradise: Princess Life (Japan)
	 || (memcmp(game_TID, "BQBJ", 4) == 0)   	// Crayon Shin-chan: Obaka Dainin Den: Susume! Kasukabe Ninja Tai! (Japan)
	 || (memcmp(game_TID, "BUCJ", 4) == 0)   	// Crayon Shin-chan: Shock Gahn!: Densetsu o Yobu Omake Daiketsusen!! (Japan)
	 || (memcmp(game_TID, "BDNJ", 4) == 0)   	// Cross Treasures (Japan)
	 || (memcmp(game_TID, "TPGJ", 4) == 0)   	// Dengeki Gakuen RPG: Cross of Venus Special (Japan)
	 || (memcmp(game_TID, "BLEJ", 4) == 0)   	// Digimon Story: Lost Evolution (Japan)
	 || (memcmp(game_TID, "TBFJ", 4) == 0)   	// Digimon Story: Super Xros Wars: Blue (Japan)
	 || (memcmp(game_TID, "TLTJ", 4) == 0)   	// Digimon Story: Super Xros Wars: Red (Japan)
	 || (memcmp(game_TID, "BVIJ", 4) == 0)   	// Dokonjou Shougakusei Bon Biita: Hadaka no Choujou Ketsusen!!: Biita vs Dokuro Dei! (Japan)
	 || (memcmp(game_TID, "TDBJ", 4) == 0)   	// Dragon Ball Kai: Ultimate Butou Den (Japan)
	 || (memcmp(game_TID, "B2JJ", 4) == 0)   	// Dragon Quest Monsters: Joker 2: Professional (Japan)
	 || (memcmp(game_TID, "YVKK", 4) == 0)   	// DS Vitamin: Widaehan Bapsang: Malhaneun! Geongangyori Giljabi (Korea)
	 || (memcmp(game_TID, "B3LJ", 4) == 0)   	// Eigo de Tabisuru: Little Charo (Japan)
	 || (memcmp(game_TID, "VL3J", 4) == 0)   	// Elminage II: Sousei no Megami to Unmei no Daichi: DS Remix (Japan)
	 || (memcmp(game_TID, "THMJ", 4) == 0)   	// FabStyle (Japan)
	 || (memcmp(game_TID, "VI2J", 4) == 0)   	// Fire Emblem: Shin Monshou no Nazo: Hikari to Kage no Eiyuu (Japan)
	 || (memcmp(game_TID, "BFPJ", 4) == 0)   	// Fresh PreCure!: Asobi Collection (Japan)
	 || (memcmp(game_TID, "B4FJ", 4) == 0)   	// Fushigi no Dungeon: Fuurai no Shiren 4: Kami no Hitomi to Akuma no Heso (Japan)
	 || (memcmp(game_TID, "B5FJ", 4) == 0)   	// Fushigi no Dungeon: Fuurai no Shiren 5: Fortune Tower to Unmei no Dice (Japan)
	 || (memcmp(game_TID, "BG3J", 4) == 0)   	// G.G Series Collection+ (Japan)
	 || (memcmp(game_TID, "BRQJ", 4) == 0)   	// Gendai Daisenryaku DS: Isshoku Sokuhatsu, Gunji Balance Houkai (Japan)
	 || (memcmp(game_TID, "VMMJ", 4) == 0)   	// Gokujou!! Mecha Mote Iinchou: MM My Best Friend! (Japan)
	 || (memcmp(game_TID, "BM7J", 4) == 0)   	// Gokujou!! Mecha Mote Iinchou: MM Town de Miracle Change! (Japan)
	 || (memcmp(game_TID, "BXOJ", 4) == 0)   	// Gyakuten Kenji 2 (Japan)
	 || (memcmp(game_TID, "BQFJ", 4) == 0)   	// HeartCatch PreCure!: Oshare Collection (Japan)
	 || (memcmp(game_TID, "AWIK", 4) == 0)   	// Hotel Duskui Bimil (Korea)
	 || (memcmp(game_TID, "YHGJ", 4) == 0)   	// Houkago Shounen (Japan)
	 || (memcmp(game_TID, "BRYJ", 4) == 0)   	// Hudson x GReeeeN: Live! DeeeeS! (Japan)
	 || (memcmp(game_TID, "YG4K", 4) == 0)   	// Hwansangsuhojeon: Tierkreis (Korea)
	 || (memcmp(game_TID, "BZ2J", 4) == 0)   	// Imasugu Tsukaeru Mamechishiki: Quiz Zatsugaku-ou DS (Japan)
	 || (memcmp(game_TID, "BEZJ", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: Bomber (Japan)
	 || (memcmp(game_TID, "BE8J", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: Spark (Japan)
	// || (memcmp(game_TID, "BOEJ", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: The Ogre (Japan) (Patched by nds-bootstrap)
	 || (memcmp(game_TID, "BJKJ", 4) == 0)   	// Ippan Zaidan Houjin Nihon Kanji Shuujukudo Kentei Kikou Kounin: Kanjukuken DS (Japan)
	 || (memcmp(game_TID, "BIMJ", 4) == 0)   	// Iron Master: The Legendary Blacksmith (Japan)
	 || (memcmp(game_TID, "CDOK", 4) == 0)   	// Iron Master: Wanggugui Yusangwa Segaeui Yeolsoe (Korea)
	 || (memcmp(game_TID, "YROK", 4) == 0)   	// Isanghan Naraui Princess (Korea)
	 || (memcmp(game_TID, "BRGJ", 4) == 0)   	// Ishin no Arashi: Shippuu Ryouma Den (Japan)
	 || (memcmp(game_TID, "UXBP", 4) == 0)   	// Jam with the Band (Europe)
	 || (memcmp(game_TID, "YEOK", 4) == 0)   	// Jeoldaepiryo: Yeongsugeo 1000 DS (Korea)
	 || (memcmp(game_TID, "YE9K", 4) == 0)   	// Jeoldaeuwi: Yeongdaneo 1900 DS (Korea)
	 || (memcmp(game_TID, "B2OK", 4) == 0)   	// Jeongukmin Model Audition Superstar DS (Korea)
	 || (memcmp(game_TID, "YRCK", 4) == 0)   	// Jjangguneun Monmallyeo: Cinemaland Chalkak Chalkak Daesodong! (Korea)
	 || (memcmp(game_TID, "CL4K", 4) == 0)   	// Jjangguneun Monmallyeo: Mallangmallang Gomuchalheuk Daebyeonsin! (Korea)
	 || (memcmp(game_TID, "BOBJ", 4) == 0)   	// Kaidan Restaurant: Ura Menu 100-sen (Japan)
	 || (memcmp(game_TID, "TK2J", 4) == 0)   	// Kaidan Restaurant: Zoku! Shin Menu 100-sen (Japan)
	 || (memcmp(game_TID, "BA7J", 4) == 0)   	// Kaibou Seirigaku DS: Touch de Hirogaru! Jintai no Kouzou to Kinou (Japan)
	 || (memcmp(game_TID, "BKXJ", 4) == 0)   	// Kaijuu Busters (Japan)
	 || (memcmp(game_TID, "BYVJ", 4) == 0)   	// Kaijuu Busters Powered (Japan)
	 || (memcmp(game_TID, "TGKJ", 4) == 0)   	// Kaizoku Sentai Gokaiger: Atsumete Henshin! 35 Sentai! (Japan)
	 || (memcmp(game_TID, "B8RJ", 4) == 0)   	// Kamen Rider Battle: GanbaRide: Card Battle Taisen (Japan)
	 || (memcmp(game_TID, "BKHJ", 4) == 0)   	// Kamonohashikamo.: Aimai Seikatsu no Susume (Japan)
	 || (memcmp(game_TID, "BKPJ", 4) == 0)   	// Kanshuu: Shuukan Pro Wrestling: Pro Wrestling Kentei DS (Japan)
	 || (memcmp(game_TID, "BEKJ", 4) == 0)   	// Kanzen Taiou Saishin Kako Mondai: Nijishiken Taisaku: Eiken Kanzenban (Japan)
	 || (memcmp(game_TID, "BQJJ", 4) == 0)   	// Kawaii Koneko DS 3 (Japan)
	 || (memcmp(game_TID, "BKKJ", 4) == 0)   	// Keroro RPG: Kishi to Musha to Densetsu no Kaizoku (Japan)
	 || (memcmp(game_TID, "BKSJ", 4) == 0)   	// Keshikasu-kun: Battle Kasu-tival (Japan)
	 || (memcmp(game_TID, "BKTJ", 4) == 0)   	// Kimi ni Todoke: Sodateru Omoi (Japan)
	 || (memcmp(game_TID, "TK9J", 4) == 0)   	// Kimi ni Todoke: Special (Japan)
	 || (memcmp(game_TID, "TKTJ", 4) == 0)   	// Kimi ni Todoke: Tsutaeru Kimochi (Japan)
	 || (memcmp(game_TID, "CKDJ", 4) == 0)   	// Kindaichi Shounen no Jikenbo: Akuma no Satsujin Koukai (Japan)
	 || (memcmp(game_TID, "VCGJ", 4) == 0)   	// Kirakira Rhythm Collection (Japan)
	 || (memcmp(game_TID, "BCKJ", 4) == 0)   	// Kochira Katsushika Ku Kameari Kouen Mae Hashutsujo: Kateba Tengoku! Makereba Jigoku!: Ryoutsu-ryuu Ikkakusenkin Daisakusen! (Japan)
	 || (memcmp(game_TID, "BCXJ", 4) == 0)   	// Kodawari Saihai Simulation: Ochanoma Pro Yakyuu DS: 2010 Nendo Ban (Japan)
	 || (memcmp(game_TID, "VKPJ", 4) == 0)   	// Korg DS-10+ Synthesizer Limited Edition (Japan)
	 || (memcmp(game_TID, "BZMJ", 4) == 0)   	// Korg M01 Music Workstation (Japan)
	 || (memcmp(game_TID, "BTAJ", 4) == 0)   	// Lina no Atelier: Strahl no Renkinjutsushi (Japan)
	 || (memcmp(game_TID, "BCDK", 4) == 0)   	// Live-On Card Live-R DS (Korea)
	 || (memcmp(game_TID, "VLIP", 4) == 0)   	// Lost Identities (Europe)
	 || (memcmp(game_TID, "BOXJ", 4) == 0)   	// Love Plus+ (Japan)
	 || (memcmp(game_TID, "BL3J", 4) == 0)   	// Lupin Sansei: Shijou Saidai no Zunousen (Japan)
	 || (memcmp(game_TID, "YNOK", 4) == 0)   	// Mabeopcheonjamun DS (Korea)
	 || (memcmp(game_TID, "BCJK", 4) == 0)   	// Mabeopcheonjamun DS 2 (Korea)
	 || (memcmp(game_TID, "ANMK", 4) == 0)   	// Maeilmaeil Deoukdeo!: DS Dunoe Training (Korea)
	 || (memcmp(game_TID, "TCYE", 4) == 0)   	// Mama's Combo Pack: Volume 1 (USA)
	 || (memcmp(game_TID, "TCZE", 4) == 0)   	// Mama's Combo Pack: Volume 2 (USA)
	 || (memcmp(game_TID, "ANMK", 4) == 0)   	// Marie-Antoinette and the American War of Independence: Episode 1: The Brotherhood of the Wolf (Europe)
	 || (memcmp(game_TID, "BA5K", 4) == 0)   	// Mario & Luigi RPG: Siganui Partner (Korea)
	 || (memcmp(game_TID, "C6OJ", 4) == 0)   	// Medarot DS: Kabuto Ver. (Japan)
	 || (memcmp(game_TID, "BQWJ", 4) == 0)   	// Medarot DS: Kuwagata Ver. (Japan)
	 || (memcmp(game_TID, "BBJJ", 4) == 0)   	// Metal Fight Beyblade: Baku Shin Susanoo Shuurai! (Japan)
	 || (memcmp(game_TID, "TKNJ", 4) == 0)   	// Meitantei Conan: Aoki Houseki no Rondo (Japan)
	 || (memcmp(game_TID, "TMKJ", 4) == 0)   	// Meitantei Conan: Kako Kara no Zensou Kyoku (Japan)
	 || (memcmp(game_TID, "TMXJ", 4) == 0)   	// Metal Max 2: Reloaded (Japan)
	 || (memcmp(game_TID, "C34J", 4) == 0)   	// Mini Yonku DS (Japan)
	 || (memcmp(game_TID, "BWCJ", 4) == 0)   	// Minna no Conveni (Japan)
	 || (memcmp(game_TID, "BQUJ", 4) == 0)   	// Minna no Suizokukan (Japan)
	 || (memcmp(game_TID, "BQVJ", 4) == 0)   	// Minna to Kimi no Piramekino! (Japan)
	 || (memcmp(game_TID, "B2WJ", 4) == 0)   	// Moe Moe 2-ji Taisen(ryaku) Two: Yamato Nadeshiko (Japan)
	 || (memcmp(game_TID, "BWRJ", 4) == 0)   	// Momotarou Dentetsu: World (Japan)
	 || (memcmp(game_TID, "CZZK", 4) == 0)   	// Monmallineun 3-gongjuwa Hamkkehaneun: Geurimyeonsang Yeongdaneo Amgibeop (Korea)
	 || (memcmp(game_TID, "B3IJ", 4) == 0)   	// Motto! Stitch! DS: Rhythm de Rakugaki Daisakusen (Japan)
	 || (memcmp(game_TID, "C6FJ", 4) == 0)   	// Mugen no Frontier Exceed: Super Robot Taisen OG Saga (Japan)
	 || (memcmp(game_TID, "B74J", 4) == 0)   	// Nanashi no Geemu Me (Japan)
	 || (memcmp(game_TID, "TNRJ", 4) == 0)   	// Nora to Toki no Koubou: Kiri no Mori no Majo (Japan)
	 || (memcmp(game_TID, "YNRK", 4) == 0)   	// Naruto Jilpungjeon: Daenantu! Geurimja Bunsinsul (Korea)
	 || (memcmp(game_TID, "BKJJ", 4) == 0)   	// Nazotte Oboeru: Otona no Kanji Renshuu: Kaiteiban (Japan)
	 || (memcmp(game_TID, "BPUJ", 4) == 0)   	// Nettou! Powerful Koushien (Japan)
	 || (memcmp(game_TID, "TJ7J", 4) == 0)   	// New Horizon: English Course 3 (Japan)
	 || (memcmp(game_TID, "TJ8J", 4) == 0)   	// New Horizon: English Course 2 (Japan)
	 || (memcmp(game_TID, "TJ9J", 4) == 0)   	// New Horizon: English Course 1 (Japan)
	 || (memcmp(game_TID, "BETJ", 4) == 0)   	// Nihon Keizai Shinbunsha Kanshuu: Shiranai Mama dewa Son wo Suru: 'Mono ya Okane no Shikumi' DS (Japan)
	 || (memcmp(game_TID, "YCUP", 4) == 0)   	// Nintendo Presents: Crossword Collection (Europe)
	 || (memcmp(game_TID, "B2KJ", 4) == 0)   	// Ni no Kuni: Shikkoku no Madoushi (Japan)
	 || (memcmp(game_TID, "BNCJ", 4) == 0)   	// Nodame Cantabile: Tanoshii Ongaku no Jikan Desu (Japan)
	 || (memcmp(game_TID, "CQKP", 4) == 0)   	// NRL Mascot Mania (Australia)
	 || (memcmp(game_TID, "BO4J", 4) == 0)   	// Ochaken no Heya DS 4 (Japan)
	 || (memcmp(game_TID, "BOYJ", 4) == 0)   	// Odoru Daisousa-sen: The Game: Sensuikan ni Sennyuu Seyo! (Japan)
	 || (memcmp(game_TID, "B62J", 4) == 0)   	// Okaeri! Chibi-Robo!: Happy Rich Oosouji! (Japan)
	 || (memcmp(game_TID, "TGBJ", 4) == 0)   	// One Piece Gigant Battle 2: Shin Sekai (Japan)
	 || (memcmp(game_TID, "BOKJ", 4) == 0)   	// Ookami to Koushinryou: Umi o Wataru Kaze (Japan)
	 || (memcmp(game_TID, "TKDJ", 4) == 0)   	// Ore-Sama Kingdom: Koi mo Manga mo Debut o Mezase! Doki Doki Love Lesson (Japan)
	 || (memcmp(game_TID, "TFTJ", 4) == 0)   	// Original Story from Fairy Tail: Gekitotsu! Kardia Daiseidou (Japan)
	 || (memcmp(game_TID, "BHQJ", 4) == 0)   	// Otona no Renai Shousetsu: DS Harlequin Selection (Japan)
	 || (memcmp(game_TID, "BIPJ", 4) == 0)   	// Pen1 Grand Prix: Penguin no Mondai Special (Japan)
	 || (memcmp(game_TID, "BO9J", 4) == 0)   	// Penguin no Mondai: The World (Japan)
	 || (memcmp(game_TID, "B42J", 4) == 0)   	// Pet Shop Monogatari DS 2 (Japan)
	 || (memcmp(game_TID, "BVGE", 4) == 0)   	// Petz: Bunnyz Bunch (USA)
	 || (memcmp(game_TID, "BLLE", 4) == 0)   	// Petz: Catz Playground (USA)
	 || (memcmp(game_TID, "BUFE", 4) == 0)   	// Petz: Puppyz & Kittenz (USA)
	 || (memcmp(game_TID, "VFBE", 4) == 0)   	// Petz Fantasy: Moonlight Magic (USA)
	 || (memcmp(game_TID, "VTPV", 4) == 0)   	// Phineas and Ferb: 2 Disney Games (Europe)
	 || (memcmp(game_TID, "B5VE", 4) == 0)   	// Phineas and Ferb: Across the 2nd Dimension (USA)
	 || (memcmp(game_TID, "YFTK", 4) == 0)   	// Pokemon Bulgasaui Dungeon: Siganui Tamheomdae (Korea)
	 || (memcmp(game_TID, "YFYK", 4) == 0)   	// Pokemon Bulgasaui Dungeon: Eodumui Tamheomdae (Korea)
	 || (memcmp(game_TID, "BPPJ", 4) == 0)   	// PostPet DS: Yumemiru Momo to Fushigi no Pen (Japan)
	 || (memcmp(game_TID, "BONJ", 4) == 0)   	// Powerful Golf (Japan)
	 || (memcmp(game_TID, "VPTJ", 4) == 0)   	// Power Pro Kun Pocket 12 (Japan)
	 || (memcmp(game_TID, "VPLJ", 4) == 0)   	// Power Pro Kun Pocket 13 (Japan)
	 || (memcmp(game_TID, "VP4J", 4) == 0)   	// Power Pro Kun Pocket 14 (Japan)
	 || (memcmp(game_TID, "B2YK", 4) == 0)   	// Ppiyodamari DS (Korea)
	 || (memcmp(game_TID, "B4NK", 4) == 0)   	// Princess Angel: Baeguiui Cheonsa (Korea)
	 || (memcmp(game_TID, "C4WK", 4) == 0)   	// Princess Bakery (Korea)
	 || (memcmp(game_TID, "CP4K", 4) == 0)   	// Princess Maker 4: Special Edition (Korea)
	 || (memcmp(game_TID, "C29J", 4) == 0)   	// Pro Yakyuu Famista DS 2009 (Japan)
	 || (memcmp(game_TID, "BF2J", 4) == 0)   	// Pro Yakyuu Famista DS 2010 (Japan)
	 || (memcmp(game_TID, "B89J", 4) == 0)   	// Pro Yakyuu Team o Tsukurou! 2 (Japan)
	 || (memcmp(game_TID, "BU9J", 4) == 0)   	// Pucca: Power Up (Europe)
	 || (memcmp(game_TID, "TP4J", 4) == 0)   	// Puyo Puyo!!: Puyopuyo 20th Anniversary (Japan)
	 || (memcmp(game_TID, "BYOJ", 4) == 0)   	// Puyo Puyo 7 (Japan)
	 || (memcmp(game_TID, "BHXJ", 4) == 0)   	// Quiz! Hexagon II (Japan)
	 || (memcmp(game_TID, "BQ2J", 4) == 0)   	// Quiz Magic Academy DS: Futatsu no Jikuuseki (Japan)
	 || (memcmp(game_TID, "YRBK", 4) == 0)   	// Ragnarok DS (Korea)
	 || (memcmp(game_TID, "TEDJ", 4) == 0)   	// Red Stone DS: Akaki Ishi ni Michibikareshi Mono-tachi (Japan)
	 || (memcmp(game_TID, "B35J", 4) == 0)   	// Rekishi Simulation Game: Sangokushi DS 3 (Japan)
	 || (memcmp(game_TID, "BUKJ", 4) == 0)   	// Rekishi Taisen: Gettenka: Tenkaichi Battle Royale (Japan)
	 || (memcmp(game_TID, "YLZK", 4) == 0)   	// Rhythm Sesang (Korea)
	 || (memcmp(game_TID, "BKMJ", 4) == 0)   	// Rilakkuma Rhythm: Mattari Kibun de Dararan Ran (Japan)
	 || (memcmp(game_TID, "B6XJ", 4) == 0)   	// Rockman EXE: Operate Shooting Star (Japan)
	 || (memcmp(game_TID, "V29J", 4) == 0)   	// RPG Tkool DS (Japan)
	 || (memcmp(game_TID, "VEBJ", 4) == 0)   	// RPG Tsukuru DS+: Create The New World (Japan)
	 || (memcmp(game_TID, "ARFK", 4) == 0)   	// Rune Factory: Sinmokjjangiyagi (Korea)
	// || (memcmp(game_TID, "CSGJ", 4) == 0)   	// SaGa 2: Hihou Densetsu: Goddess of Destiny (Japan) (Patched by nds-bootstrap)
	 || (memcmp(game_TID, "BZ3J", 4) == 0)   	// SaGa 3: Jikuu no Hasha: Shadow or Light (Japan)
	 || (memcmp(game_TID, "CBEJ", 4) == 0)   	// Saibanin Suiri Game: Yuuzai x Muzai (Japan)
	 || (memcmp(game_TID, "B59J", 4) == 0)   	// Sakusaku Jinkou Kokyuu Care Training DS (Japan)
	 || (memcmp(game_TID, "BSWJ", 4) == 0)   	// Saka Tsuku DS: World Challenge 2010 (Japan)
	 || (memcmp(game_TID, "B3GJ", 4) == 0)   	// SD Gundam Sangoku Den: Brave Battle Warriors: Shin Militia Taisen (Japan)
	 || (memcmp(game_TID, "B7XJ", 4) == 0)   	// Seitokai no Ichizon: DS Suru Seitokai (Japan)
	 || (memcmp(game_TID, "CQ2J", 4) == 0)   	// Sengoku Spirits: Gunshi Den (Japan)
	 || (memcmp(game_TID, "CQ3J", 4) == 0)   	// Sengoku Spirits: Moushou Den (Japan)
	 || (memcmp(game_TID, "YR4J", 4) == 0)   	// Sengoku Spirits: Shukun Den (Japan)
	 || (memcmp(game_TID, "B5GJ", 4) == 0)   	// Shin Sengoku Tenka Touitsu: Gunyuu-tachi no Souran (Japan)
	 || (memcmp(game_TID, "C36J", 4) == 0)   	// Sloane to MacHale no Nazo no Story (Japan)
	 || (memcmp(game_TID, "B2QJ", 4) == 0)   	// Sloane to MacHale no Nazo no Story 2 (Japan)
	 || (memcmp(game_TID, "A3YK", 4) == 0)   	// Sonic Rush Adventure (Korea)
	 || (memcmp(game_TID, "TFLJ", 4) == 0)   	// Sora no Otoshimono Forte: Dreamy Season (Japan)
	 || (memcmp(game_TID, "YW4K", 4) == 0)   	// Spectral Force: Genesis (Korea)
	 || (memcmp(game_TID, "B22J", 4) == 0)   	// Strike Witches 2: Iyasu, Naosu, Punipuni Suru (Japan)
	 || (memcmp(game_TID, "BYQJ", 4) == 0)   	// Suisui Physical Assessment Training DS (Japan)
	 || (memcmp(game_TID, "TPQJ", 4) == 0)   	// Suite PreCure: Melody Collection (Japan)
	 || (memcmp(game_TID, "CS7J", 4) == 0)   	// Summon Night X: Tears Crown (Japan)
	 || (memcmp(game_TID, "C2YJ", 4) == 0)   	// Supa Robo Gakuen (Japan) Nazotoki Adventure (Japan)
	 || (memcmp(game_TID, "BRWJ", 4) == 0)   	// Super Robot Taisen L (Japan)
	 || (memcmp(game_TID, "BROJ", 4) == 0)   	// Super Robot Taisen OG Saga: Masou Kishin: The Lord of Elemental (Japan)
	 || (memcmp(game_TID, "C5IJ", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-1-shuu: Nazotoki Sekai Isshuu Ryokou (Japan)
	 || (memcmp(game_TID, "C52J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-2-shuu: Ginga Oudan (Japan)
	 || (memcmp(game_TID, "BQ3J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-3-shuu: Fushigi no Kuni no Nazotoki Otogibanashi (Japan)
	 || (memcmp(game_TID, "BQ4J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-4-shuu: Time Machine no Nazotoki Daibouken (Japan)
	 || (memcmp(game_TID, "B3DJ", 4) == 0)   	// Taiko no Tatsujin DS: Dororon! Yookai Daikessen!! (Japan)
	 || (memcmp(game_TID, "B7KJ", 4) == 0)   	// Tamagotch no Narikiri Challenge (Japan)
	 || (memcmp(game_TID, "BGVJ", 4) == 0)   	// Tamagotch no Narikiri Channel (Japan)
	 || (memcmp(game_TID, "BG5J", 4) == 0)   	// Tamagotch no Pichi Pichi Omisetchi (Japan)
	 || (memcmp(game_TID, "TGCJ", 4) == 0)   	// Tamagotchi Collection (Japan)
	 || (memcmp(game_TID, "BQ9J", 4) == 0)   	// Tekipaki Kyuukyuu Kyuuhen Training DS (Japan)
	 || (memcmp(game_TID, "B5KJ", 4) == 0)   	// Tenkaichi: Sengoku Lovers DS (Japan)
	 || (memcmp(game_TID, "TENJ", 4) == 0)   	// Tennis no Ouji-sama: Gyutto! Dokidoki Survival: Umi to Yama no Love Passion (Japan)
	 || (memcmp(game_TID, "BTGJ", 4) == 0)   	// Tennis no Ouji-sama: Motto Gakuensai no Ouji-sama: More Sweet Edition (Japan)
	 || (memcmp(game_TID, "VIMJ", 4) == 0)   	// The Idolm@ster: Dearly Stars (Japan)
	 || (memcmp(game_TID, "B6KP", 4) == 0)   	// Tinker Bell + Tinker Bell and the Lost Treasure (Europe)
	 || (memcmp(game_TID, "TKGJ", 4) == 0)   	// Tobidase! Kagaku-kun: Chikyuu Daitanken! Nazo no Chinkai Seibutsu ni Idome! (Japan)
	 || (memcmp(game_TID, "CT5K", 4) == 0)   	// TOEIC DS: Haru 10-bun Yakjeomgeukbok +200 (Korea)
	 || (memcmp(game_TID, "AEYK", 4) == 0)   	// TOEIC Test DS Training (Korea)
	 || (memcmp(game_TID, "BT5J", 4) == 0)   	// TOEIC Test Super Coach@DS (Japan)
	 || (memcmp(game_TID, "TQ5J", 4) == 0)   	// Tokumei Sentai Go Busters (Japan)
	 || (memcmp(game_TID, "CVAJ", 4) == 0)   	// Tokyo Twilight Busters: Kindan no Ikenie Teito Jigokuhen (Japan)
	 || (memcmp(game_TID, "CZXK", 4) == 0)   	// Touch Man to Man: Gichoyeongeo (Korea)
	 || (memcmp(game_TID, "BUQJ", 4) == 0)   	// Treasure Report: Kikai Jikake no Isan (Japan)
	 || (memcmp(game_TID, "C2VJ", 4) == 0)   	// Tsukibito (Japan)
	 || (memcmp(game_TID, "BH6J", 4) == 0)   	// TV Anime Fairy Tail: Gekitou! Madoushi Kessen (Japan)
	 || (memcmp(game_TID, "CUHJ", 4) == 0)   	// Umihara Kawase Shun: Second Edition Kanzen Ban (Japan)
	 || (memcmp(game_TID, "TBCJ", 4) == 0)   	// Usavich: Game no Jikan (Japan)
	 || (memcmp(game_TID, "BPOJ", 4) == 0)   	// Utacchi (Japan)
	 || (memcmp(game_TID, "BXPJ", 4) == 0)   	// Winnie the Pooh: Kuma no Puu-san: 100 Acre no Mori no Cooking Book (Japan)
	 || (memcmp(game_TID, "BWYJ", 4) == 0)   	// Wizardry: Boukyaku no Isan (Japan)
	 || (memcmp(game_TID, "BWZJ", 4) == 0)   	// Wizardry: Inochi no Kusabi (Japan)
	 || (memcmp(game_TID, "BWWJ", 4) == 0)   	// WiZmans World (Japan)
	 || (memcmp(game_TID, "BYNJ", 4) == 0)   	// Yamakawa Shuppansha Kanshuu: Shousetsu Nihonshi B: Shin Sougou Training Plus (Japan)
	 || (memcmp(game_TID, "BYSJ", 4) == 0)   	// Yamakawa Shuppansha Kanshuu: Shousetsu Sekaishi B: Shin Sougou Training Plus (Japan)
	 || (memcmp(game_TID, "B5DJ", 4) == 0)   	// Yamanote-sen Meimei 100 Shuunen Kinen: Densha de Go!: Tokubetsu Hen: Fukkatsu! Shouwa no Yamanote-sen (Japan)
	 || (memcmp(game_TID, "BYMJ", 4) == 0)   	// Yumeiro Patissiere: My Sweets Cooking (Japan)
	 || (memcmp(game_TID, "BZQJ", 4) == 0)   	// Zac to Ombra: Maboroshi no Yuuenchi (Japan)
	 || (memcmp(game_TID, "BZBJ", 4) == 0)) {	// Zombie Daisuki (Japan)
		return 1;
	} else {
		static const char ap_list[][4] = {
			"YBN",	// 100 Classic Books
			"VAL",	// Alice in Wonderland
			"VAA",	// Art Academy
			"C7U",	// Battle of Giants: Dragons
			"BIG",	// Battle of Giants: Mutant Insects
			"BBU",	// Beyblade: Metal Fusion
			"BRZ",	// Beyblade: Metal Masters
			"YBU",	// Blue Dragon: Awakened Shadow
			"VKH",	// Brainstorm Series: Treasure Chase
			"BDU",	// C.O.P.: The Recruit
			"BDY",	// Call of Duty: Black Ops
			"TCM",	// Camping Mama: Outdoor Adventures
			"VCM",	// Camp Rock: The Final Jam
			"BQN",	// Captain America: Super Soldier
			"B2B",	// Captain Tsubasa
			"VCA",	// Cars 2
			"VMY",	// Chronicles of Mystery: The Secret Tree of Life
			"YQU",	// Chrono Trigger
			"CY9",	// Club Penguin: EPF: Herbert's Revenge
			"BQ8",	// Crafting Mama
			"VAO",	// Crime Lab: Body of Evidence
			"BD2",	// Deca Sports DS
			"BDE",	// Dementium II
			"BDB",	// Dragon Ball: Origins 2
			"YV5",	// Dragon Quest V: Hand of the Heavenly Bride
			"YVI",	// Dragon Quest VI: Realms of Revelation
			"YDQ",	// Dragon Quest IX: Sentinels of the Starry Skies
			"CJR",	// Dragon Quest Monsters: Joker 2
			"BEL",	// Element Hunters
			"BJ3",	// Etrian Odyssey III: The Drowned City
			"CFI",	// Final Fantasy Crystal Chronicles: Echoes of Time
			"BFX",	// Final Fantasy: The 4 Heroes of Light
			"VDE",	// Fossil Fighters Champions
			"BJC",	// GoldenEye 007
			// "BO5",	// Golden Sun: Dark Dawn (Patched by nds-bootstrap)
			"YGX",	// Grand Theft Auto: Chinatown Wars
			"BGT",	// Ghost Trick: Phantom Detective
			"B7H",	// Harry Potter and the Deathly Hallows: Part 1
			"BU8",	// Harry Potter and the Deathly Hallows: Part 2
			"BKU",	// Harvest Moon DS: The Tale of Two Towns
			"YEE",	// Inazuma Eleven
			"BEE",	// Inazuma Eleven 2: Blizzard
			"BEB",	// Inazuma Eleven 2: Firestorm
			"B86",	// Jewels of the Ages
			"YKG",	// Kindgom Hearts: 358/2 Days
			"BK9",	// Kindgom Hearts: Re-coded
			"VKG",	// Korg DS-10+ Synthesizer
			"BQP",	// KuruKuru Princess: Tokimeki Figure
			"YLU", 	// Last Window: The Secret of Cape West
			"BSD",	// Lufia: Curse of the Sinistrals
			"YMP",	// MapleStory DS
			"CLJ",	// Mario & Luigi: Bowser's Inside Story
			"COL",	// Mario & Sonic at the Olympic Winter Games
			"V2G",	// Mario vs. Donkey Kong: Mini-Land Mayhem!
			// "B6Z",	// Mega Man Zero Collection (Patched by nds-bootstrap)
			"BVN",	// Michael Jackson: The Experience
			"CHN",	// Might & Magic: Clash of Heroes
			"BNQ",	// Murder in Venice
			"BFL",	// MySims: Sky Heroes
			"CNS",	// Naruto Shippuden: Naruto vs Sasuke
			"BSK",	// Nine Hours: Nine Persons: Nine Doors
			"BOJ",	// One Piece: Gigant Battle!
			"BOO",	// Ookami Den
			"VFZ",	// Petz: Fantasy
			"BNR",	// Petz: Nursery
			"B3U",	// Petz: Nursery 2
			"C24",	// Phantasy Star 0
			"BZF",	// Phineas and Ferb: Across the 2nd Dimension
			"VPF",	// Phineas and Ferb: Ride Again
			// "IPK",	// Pokemon HeartGold Version (Patched by nds-bootstrap)
			// "IPG",	// Pokemon SoulSilver Version (Patched by nds-bootstrap)
			"IRA",	// Pokemon Black Version
			"IRB",	// Pokemon White Version
			"IRE",	// Pokemon Black Version 2
			"IRD",	// Pokemon White Version 2
			"VPY",	// Pokemon Conquest
			"B3R",	// Pokemon Ranger: Guardian Signs
			"VPP",	// Prince of Persia: The Forgotten Sands
			"BLF",	// Professor Layton and the Last Specter
			"C3J",	// Professor Layton and the Unwound Future
			"BKQ",	// Pucca: Power Up
			"VRG",	// Rabbids Go Home: A Comedy Adventure
			// "BRJ",	// Radiant Historia (Patched by nds-bootstrap)
			"B3X",	// River City: Soccer Hooligans
			"BRM",	// Rooms: The Main Building
			"TDV",	// Shin Megami Tensei: Devil Survivor 2
			"BMT",	// Shin Megami Tensei: Strange Journey
			// "VCD",	// Solatorobo: Red the Hunter (Patched by nds-bootstrap)
			"BXS",	// Sonic Colors
			"VSN",	// Sonny with a Chance
			"B2U",	// Sports Collection
			"CLW",	// Star Wars: The Clone Wars: Jedi Alliance
			// "AZL",	// Style Savvy (Patched by nds-bootstrap)
			"BH2",	// Super Scribblenauts
			"B6T",	// Tangled
			"B4T",	// Tetris Party Deluxe
			"BKI",	// The Legend of Zelda: Spirit Tracks
			"VS3",	// The Sims 3
			"BZU",	// The Smurfs
			"TR2",	// The Smurfs 2
			"BS8",	// The Sorcerer's Apprentice
			"BTU",	// Tinker Bell and the Great Fairy Rescue
			"CCU",	// Tomodachi Collection
			"VT3",	// Toy Story 3
			"VTE",	// TRON: Evolution
			"B3V",	// Vampire Moon: The Mystery of the Hidden Sun
			"BW4",	// Wizards of Waverly Place: Spellbound
			"BYX",	// Yu-Gi-Oh! 5D's: World Championship 2010: Reverse of Arcadia
			"BYY",	// Yu-Gi-Oh! 5D's: World Championship 2011: Over the Nexus
		};

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(ap_list)/sizeof(ap_list[0]); i++) {
			if (memcmp(game_TID, ap_list[i], 3) == 0) {
				// Found a match.
				return 1;
				break;
			}
		}

		static const char ap_list2[][4] = {
			"VID",	// Imagine: Resort Owner
			"TAD",	// Kirby: Mass Attack
		};
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(ap_list2)/sizeof(ap_list2[0]); i++) {
			if (memcmp(game_TID, ap_list2[i], 3) == 0) {
				// Found a match.
				return 2;
				break;
			}
		}

	}
	
	return 0;
}

sNDSBannerExt bnriconTile[8];
sNDSHeaderExt* preloadedHeaders = {NULL};
sNDSBannerExt* preloadedBannerIcons = {NULL};
bool* headerPreloaded = {NULL};
bool* bannerIconPreloaded = {NULL};
static int headerFileOffset[16] = {0};
static int bannerIconFileOffset[16] = {0};
static int lastUsedHeader = 0;
static int lastUsedBannerIcon = 0;

void allocateBannerIconsToPreload(void) {
	const int count = dsiFeatures() ? 1024 : 16;
	preloadedHeaders = new sNDSHeaderExt[count];
	preloadedBannerIcons = new sNDSBannerExt[count];
	if (dsiFeatures()) {
		headerPreloaded = new bool[count];
		bannerIconPreloaded = new bool[count];
	}
}

void resetPreloadedBannerIcons(void) {
	const int count = dsiFeatures() ? 1024 : 16;
	for (int i = 0; i < count; i++) {
		if (dsiFeatures()) {
			headerPreloaded[i] = false;
			bannerIconPreloaded[i] = false;
		} else {
			headerFileOffset[i] = -1;
			bannerIconFileOffset[i] = -1;
		}
	}
}

bool preloadedHeaderFound(const int fileOffset) {
	if (dsiFeatures()) {
		return headerPreloaded[fileOffset];
	}

	for (int i = 0; i < 16; i++) {
		if (headerFileOffset[i] == fileOffset) {
			return true;
		}
	}

	return false;
}

bool preloadedBannerIconFound(const int fileOffset) {
	if (dsiFeatures()) {
		return bannerIconPreloaded[fileOffset];
	}

	for (int i = 0; i < 16; i++) {
		if (bannerIconFileOffset[i] == fileOffset) {
			return true;
		}
	}

	return false;
}

sNDSHeaderExt* getPreloadedHeader(const int fileOffset) {
	if (dsiFeatures()) {
		return &preloadedHeaders[fileOffset];
	}

	for (int i = 0; i < 16; i++) {
		if (headerFileOffset[i] == fileOffset) {
			return &preloadedHeaders[i];
		}
	}

	const int currentUsedHeader = lastUsedHeader;
	lastUsedHeader++;
	if (lastUsedHeader == 16) lastUsedHeader = 0;

	headerFileOffset[currentUsedHeader] = fileOffset;

	return &preloadedHeaders[currentUsedHeader];
}

sNDSBannerExt* getPreloadedBannerIcon(const int fileOffset) {
	if (dsiFeatures()) {
		return &preloadedBannerIcons[fileOffset];
	}

	for (int i = 0; i < 16; i++) {
		if (bannerIconFileOffset[i] == fileOffset) {
			return &preloadedBannerIcons[i];
		}
	}

	const int currentUsedBannerIcon = lastUsedBannerIcon;
	lastUsedBannerIcon++;
	if (lastUsedBannerIcon == 16) lastUsedBannerIcon = 0;

	bannerIconFileOffset[currentUsedBannerIcon] = fileOffset;

	return &preloadedBannerIcons[currentUsedBannerIcon];
}

// bnriconframeseq[]
static u16 bnriconframeseq[8][64] = {0x0000};

// bnriconframenum[]
int bnriconPalLoaded[8] = {0};
int bnriconPalLine[8] = {0};
int bnriconPalLinePrev[8] = {0};
int bnriconframenumY[8] = {0};
int bnriconframenumYPrev[8] = {0};
int bannerFlip[8] = {GL_FLIP_NONE};
int bannerFlipPrev[8] = {GL_FLIP_NONE};

// bnriconisDSi[]
bool isValid[8] = {false};
bool isTwlm[8] = {false};
bool isDirectory[8] = {false};
int bnrRomType[8] = {0};
bool bnriconisDSi[8] = {false};
int bnrWirelessIcon[8] = {0}; // 0 = None, 1 = Local, 2 = WiFi
char gameTid[8][5] = {0};
u8 romVersion[8] = {0};
u8 romUnitCode[8] = {0};
u32 a7mbk6[8] = {0};
bool isDSiWare[8] = {false};
bool isHomebrew[8] = {false};
bool isModernHomebrew[8] = {false};		// false == No DSi-Extended header, true == Has DSi-Extended header
bool requiresRamDisk[8] = {false};
int requiresDonorRom[8] = {0};
int customIcon[8] = {0};					// 0 = None, 1 = png, 2 = banner.bin, -1 = error
char customIconPath[256];

static u16 bannerDelayNum[8] = {0x0000};
int currentbnriconframeseq[8] = {0};

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence(int iconnum)
{
	memcpy(bnriconframeseq[iconnum], bnriconTile[iconnum].dsi_seq, 64 * sizeof(u16));
	bannerDelayNum[iconnum] = 0;
	currentbnriconframeseq[iconnum] = 0;
}

/**
 * Copy banner sequence to a different icon slot.
 * @param binFile Banner file.
 */
void copyBannerSequence(int iconnumDst, int iconnumSrc)
{
	memcpy(bnriconframeseq[iconnumDst], bnriconframeseq[iconnumSrc], 64 * sizeof(u16));
	bannerDelayNum[iconnumDst] = bannerDelayNum[iconnumSrc];
	currentbnriconframeseq[iconnumDst] = currentbnriconframeseq[iconnumSrc];
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence(int iconnum)
{
	memset(bnriconframeseq[iconnum], 0, 64 * sizeof(u16));
	bannerDelayNum[iconnum] = 0;
	currentbnriconframeseq[iconnum] = 0;
}

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
bool playBannerSequence(int iconnum)
{
	if (bnriconframeseq[iconnum][currentbnriconframeseq[iconnum] + 1] == 0x0100) {
		// Do nothing if icon isn't animated
		bnriconPalLine[iconnum] = 0;
		bnriconframenumY[iconnum] = 0;
		bannerFlip[iconnum] = GL_FLIP_NONE;
	} else {
		u16 setframeseq = bnriconframeseq[iconnum][currentbnriconframeseq[iconnum]];
		bnriconPalLine[iconnum] = SEQ_PAL(setframeseq);
		bnriconframenumY[iconnum] = SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV) {
			bannerFlip[iconnum] = GL_FLIP_H | GL_FLIP_V;
		} else if (!flipH && !flipV) {
			bannerFlip[iconnum] = GL_FLIP_NONE;
		} else if (flipH && !flipV) {
			bannerFlip[iconnum] = GL_FLIP_H;
		} else if (!flipH && flipV) {
			bannerFlip[iconnum] = GL_FLIP_V;
		}

		bool updateIcon = false;

		if (bnriconPalLinePrev[iconnum] != bnriconPalLine[iconnum]) {
			bnriconPalLinePrev[iconnum] = bnriconPalLine[iconnum];
			updateIcon = true;
		}

		if (bnriconframenumYPrev[iconnum] != bnriconframenumY[iconnum]) {
			bnriconframenumYPrev[iconnum] = bnriconframenumY[iconnum];
			updateIcon = true;
		}

		if (bannerFlipPrev[iconnum] != bannerFlip[iconnum]) {
			bannerFlipPrev[iconnum] = bannerFlip[iconnum];
			updateIcon = true;
		}

		bannerDelayNum[iconnum]++;
		if (bannerDelayNum[iconnum] >= (setframeseq & 0x00FF)) {
			bannerDelayNum[iconnum] = 0x0000;
			currentbnriconframeseq[iconnum]++;
			if (bnriconframeseq[iconnum][currentbnriconframeseq[iconnum]] == 0x0000) {
				currentbnriconframeseq[iconnum] = 0; // Reset sequence
			}
		}

		return updateIcon;
	}

	return false;
}
