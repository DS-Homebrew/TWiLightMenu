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
#include <gl2d.h>

#include "ndsheaderbanner.h"
#include "module_params.h"

static u32 arm9Sig[3][4];

extern sNDSBannerExt ndsBanner;

bool checkDsiBinaries(FILE* ndsFile) {
	sNDSHeaderExt ndsHeader;

	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);

	if (ndsHeader.unitCode == 0) {
		return true;
	}

	if (ndsHeader.arm9iromOffset < 0x8000 || ndsHeader.arm9iromOffset >= 0x20000000
	 || ndsHeader.arm7iromOffset < 0x8000 || ndsHeader.arm7iromOffset >= 0x20000000) {
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
	u16 headerCRC16 = 0;
	fseek(ndsFile, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
	fread(&headerCRC16, sizeof(u16), 1, ndsFile);

	{
		char apFixPath[256];
		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s.ips", sys().isRunFromSD() ? "sd" : "fat", filename);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s.bin", sys().isRunFromSD() ? "sd" : "fat", filename);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s-%04X.ips", sys().isRunFromSD() ? "sd" : "fat", gameTid, headerCRC16);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
			return 0;
		}

		sprintf(apFixPath, "%s:/_nds/nds-bootstrap/apFix/%s-%04X.bin", sys().isRunFromSD() ? "sd" : "fat", gameTid, headerCRC16);
		if (access(apFixPath, F_OK) == 0) {
			logPrint("AP-fix found!\n");
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
				int cmp = strcmp(buf, gameTid);
				if (cmp == 0) { // TID matches, check CRC
					tidFound = true;
					u16 crc;
					fread(&crc, 1, sizeof(crc), file);
					logPrint("TID match: %s, CRC: %04X\n", gameTid, crc);

					if (crc == 0xFFFF || crc == headerCRC16) { // CRC matches
						fclose(file);
						logPrint("AP-fix found!\n");
						return false;
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

	// Check for SDK4-5 ROMs that don't have AP measures.
	if ((memcmp(gameTid, "AZLJ", 4) == 0)   	// Girls Mode (JAP version of Style Savvy)
	 || (memcmp(gameTid, "YEEJ", 4) == 0)   	// Inazuma Eleven (Japan)
	 || (memcmp(gameTid, "CNSX", 4) == 0)   	// Naruto Shippuden: Naruto vs Sasuke (Europe)
	 || (memcmp(gameTid, "BH2J", 4) == 0)) {	// Super Scribblenauts (Japan)
		return 0;
	} else
	// Check for ROMs that have AP measures.
	if ((memcmp(gameTid, "VETP", 4) == 0)   	// 1000 Cooking Recipes from Elle a Table (Europe)
	 || (memcmp(gameTid, "CQQP", 4) == 0)   	// AFL Mascot Manor (Australia)
	 || (memcmp(gameTid, "CA5E", 4) == 0)   	// Again: Interactive Crime Novel (USA)
	 || (memcmp(gameTid, "TAKJ", 4) == 0)   	// All Kamen Rider: Rider Generation (Japan)
	 || (memcmp(gameTid, "BKCE", 4) == 0)   	// America's Test Kitchen: Let's Get Cooking (USA)
	 || (memcmp(gameTid, "A3PJ", 4) == 0)   	// Anpanman to Touch de Waku Waku Training (Japan)
	 || (memcmp(gameTid, "B2AK", 4) == 0)   	// Aranuri: Badachingudeulkkwa hamkke Mandeuneun Sesang (Korea)
	 || (memcmp(gameTid, "BB4J", 4) == 0)   	// Battle Spirits Digital Starter (Japan)
	 || (memcmp(gameTid, "CYJJ", 4) == 0)   	// Blood of Bahamut (Japan)
	 || (memcmp(gameTid, "TBSJ", 4) == 0)   	// Byoutai Seiri DS: Image Dekiru! Shikkan, Shoujou to Care (Japan)
	 || (memcmp(gameTid, "C5YJ", 4) == 0)   	// Chocobo to Mahou no Ehon: Majo to Shoujo to 5-nin no Yuusha (Japan)
	 || (memcmp(gameTid, "C6HK", 4) == 0)   	// Chuldong! Rescue Force DS (Korea)
	 || (memcmp(gameTid, "CCTJ", 4) == 0)   	// Cid to Chocobo no Fushigi na Dungeon: Toki Wasure no Meikyuu DS+ (Japan)
	 || (memcmp(gameTid, "CLPD", 4) == 0)   	// Club Penguin: Elite Penguin Force (Germany)
	 || (memcmp(gameTid, "BQ6J", 4) == 0)   	// Cocoro no Cocoron (Japan)
	 || (memcmp(gameTid, "BQIJ", 4) == 0)   	// Cookin' Idol I! My! Mine!: Game de Hirameki! Kirameki! Cooking (Japan)
	 || (memcmp(gameTid, "B3CJ", 4) == 0)   	// Cooking Mama 3 (Japan)
	 || (memcmp(gameTid, "TMCP", 4) == 0)   	// Cooking Mama World: Combo Pack: Volume 1 (Europe)
	 || (memcmp(gameTid, "TMDP", 4) == 0)   	// Cooking Mama World: Combo Pack: Volume 2 (Europe)
	 || (memcmp(gameTid, "BJ8P", 4) == 0)   	// Cooking Mama World: Hobbies & Fun (Europe)
	 || (memcmp(gameTid, "VCPJ", 4) == 0)   	// Cosmetick Paradise: Kirei no Mahou (Japan)
	 || (memcmp(gameTid, "VCTJ", 4) == 0)   	// Cosmetick Paradise: Princess Life (Japan)
	 || (memcmp(gameTid, "BQBJ", 4) == 0)   	// Crayon Shin-chan: Obaka Dainin Den: Susume! Kasukabe Ninja Tai! (Japan)
	 || (memcmp(gameTid, "BUCJ", 4) == 0)   	// Crayon Shin-chan: Shock Gahn!: Densetsu o Yobu Omake Daiketsusen!! (Japan)
	 || (memcmp(gameTid, "BDNJ", 4) == 0)   	// Cross Treasures (Japan)
	 || (memcmp(gameTid, "TPGJ", 4) == 0)   	// Dengeki Gakuen RPG: Cross of Venus Special (Japan)
	 || (memcmp(gameTid, "BLEJ", 4) == 0)   	// Digimon Story: Lost Evolution (Japan)
	 || (memcmp(gameTid, "TBFJ", 4) == 0)   	// Digimon Story: Super Xros Wars: Blue (Japan)
	 || (memcmp(gameTid, "TLTJ", 4) == 0)   	// Digimon Story: Super Xros Wars: Red (Japan)
	 || (memcmp(gameTid, "BVIJ", 4) == 0)   	// Dokonjou Shougakusei Bon Biita: Hadaka no Choujou Ketsusen!!: Biita vs Dokuro Dei! (Japan)
	 || (memcmp(gameTid, "TDBJ", 4) == 0)   	// Dragon Ball Kai: Ultimate Butou Den (Japan)
	 || (memcmp(gameTid, "B2JJ", 4) == 0)   	// Dragon Quest Monsters: Joker 2: Professional (Japan)
	 || (memcmp(gameTid, "YVKK", 4) == 0)   	// DS Vitamin: Widaehan Bapsang: Malhaneun! Geongangyori Giljabi (Korea)
	 || (memcmp(gameTid, "B3LJ", 4) == 0)   	// Eigo de Tabisuru: Little Charo (Japan)
	 || (memcmp(gameTid, "VL3J", 4) == 0)   	// Elminage II: Sousei no Megami to Unmei no Daichi: DS Remix (Japan)
	 || (memcmp(gameTid, "THMJ", 4) == 0)   	// FabStyle (Japan)
	 || (memcmp(gameTid, "VI2J", 4) == 0)   	// Fire Emblem: Shin Monshou no Nazo: Hikari to Kage no Eiyuu (Japan)
	 || (memcmp(gameTid, "BFPJ", 4) == 0)   	// Fresh PreCure!: Asobi Collection (Japan)
	 || (memcmp(gameTid, "B4FJ", 4) == 0)   	// Fushigi no Dungeon: Fuurai no Shiren 4: Kami no Hitomi to Akuma no Heso (Japan)
	 || (memcmp(gameTid, "B5FJ", 4) == 0)   	// Fushigi no Dungeon: Fuurai no Shiren 5: Fortune Tower to Unmei no Dice (Japan)
	 || (memcmp(gameTid, "BG3J", 4) == 0)   	// G.G Series Collection+ (Japan)
	 || (memcmp(gameTid, "BRQJ", 4) == 0)   	// Gendai Daisenryaku DS: Isshoku Sokuhatsu, Gunji Balance Houkai (Japan)
	 || (memcmp(gameTid, "VMMJ", 4) == 0)   	// Gokujou!! Mecha Mote Iinchou: MM My Best Friend! (Japan)
	 || (memcmp(gameTid, "BM7J", 4) == 0)   	// Gokujou!! Mecha Mote Iinchou: MM Town de Miracle Change! (Japan)
	 || (memcmp(gameTid, "BXOJ", 4) == 0)   	// Gyakuten Kenji 2 (Japan)
	 || (memcmp(gameTid, "BQFJ", 4) == 0)   	// HeartCatch PreCure!: Oshare Collection (Japan)
	 || (memcmp(gameTid, "AWIK", 4) == 0)   	// Hotel Duskui Bimil (Korea)
	 || (memcmp(gameTid, "YHGJ", 4) == 0)   	// Houkago Shounen (Japan)
	 || (memcmp(gameTid, "BRYJ", 4) == 0)   	// Hudson x GReeeeN: Live! DeeeeS! (Japan)
	 || (memcmp(gameTid, "YG4K", 4) == 0)   	// Hwansangsuhojeon: Tierkreis (Korea)
	 || (memcmp(gameTid, "BZ2J", 4) == 0)   	// Imasugu Tsukaeru Mamechishiki: Quiz Zatsugaku-ou DS (Japan)
	 || (memcmp(gameTid, "BEZJ", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: Bomber (Japan)
	 || (memcmp(gameTid, "BE8J", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: Spark (Japan)
	// || (memcmp(gameTid, "BOEJ", 4) == 0)   	// Inazuma Eleven 3: Sekai e no Chousen!!: The Ogre (Japan) (Patched by nds-bootstrap)
	 || (memcmp(gameTid, "BJKJ", 4) == 0)   	// Ippan Zaidan Houjin Nihon Kanji Shuujukudo Kentei Kikou Kounin: Kanjukuken DS (Japan)
	 || (memcmp(gameTid, "BIMJ", 4) == 0)   	// Iron Master: The Legendary Blacksmith (Japan)
	 || (memcmp(gameTid, "CDOK", 4) == 0)   	// Iron Master: Wanggugui Yusangwa Segaeui Yeolsoe (Korea)
	 || (memcmp(gameTid, "YROK", 4) == 0)   	// Isanghan Naraui Princess (Korea)
	 || (memcmp(gameTid, "BRGJ", 4) == 0)   	// Ishin no Arashi: Shippuu Ryouma Den (Japan)
	 || (memcmp(gameTid, "UXBP", 4) == 0)   	// Jam with the Band (Europe)
	 || (memcmp(gameTid, "YEOK", 4) == 0)   	// Jeoldaepiryo: Yeongsugeo 1000 DS (Korea)
	 || (memcmp(gameTid, "YE9K", 4) == 0)   	// Jeoldaeuwi: Yeongdaneo 1900 DS (Korea)
	 || (memcmp(gameTid, "B2OK", 4) == 0)   	// Jeongukmin Model Audition Superstar DS (Korea)
	 || (memcmp(gameTid, "YRCK", 4) == 0)   	// Jjangguneun Monmallyeo: Cinemaland Chalkak Chalkak Daesodong! (Korea)
	 || (memcmp(gameTid, "CL4K", 4) == 0)   	// Jjangguneun Monmallyeo: Mallangmallang Gomuchalheuk Daebyeonsin! (Korea)
	 || (memcmp(gameTid, "BOBJ", 4) == 0)   	// Kaidan Restaurant: Ura Menu 100-sen (Japan)
	 || (memcmp(gameTid, "TK2J", 4) == 0)   	// Kaidan Restaurant: Zoku! Shin Menu 100-sen (Japan)
	 || (memcmp(gameTid, "BA7J", 4) == 0)   	// Kaibou Seirigaku DS: Touch de Hirogaru! Jintai no Kouzou to Kinou (Japan)
	 || (memcmp(gameTid, "BKXJ", 4) == 0)   	// Kaijuu Busters (Japan)
	 || (memcmp(gameTid, "BYVJ", 4) == 0)   	// Kaijuu Busters Powered (Japan)
	 || (memcmp(gameTid, "TGKJ", 4) == 0)   	// Kaizoku Sentai Gokaiger: Atsumete Henshin! 35 Sentai! (Japan)
	 || (memcmp(gameTid, "B8RJ", 4) == 0)   	// Kamen Rider Battle: GanbaRide: Card Battle Taisen (Japan)
	 || (memcmp(gameTid, "BKHJ", 4) == 0)   	// Kamonohashikamo.: Aimai Seikatsu no Susume (Japan)
	 || (memcmp(gameTid, "BKPJ", 4) == 0)   	// Kanshuu: Shuukan Pro Wrestling: Pro Wrestling Kentei DS (Japan)
	 || (memcmp(gameTid, "BEKJ", 4) == 0)   	// Kanzen Taiou Saishin Kako Mondai: Nijishiken Taisaku: Eiken Kanzenban (Japan)
	 || (memcmp(gameTid, "BQJJ", 4) == 0)   	// Kawaii Koneko DS 3 (Japan)
	 || (memcmp(gameTid, "BKKJ", 4) == 0)   	// Keroro RPG: Kishi to Musha to Densetsu no Kaizoku (Japan)
	 || (memcmp(gameTid, "BKSJ", 4) == 0)   	// Keshikasu-kun: Battle Kasu-tival (Japan)
	 || (memcmp(gameTid, "BKTJ", 4) == 0)   	// Kimi ni Todoke: Sodateru Omoi (Japan)
	 || (memcmp(gameTid, "TK9J", 4) == 0)   	// Kimi ni Todoke: Special (Japan)
	 || (memcmp(gameTid, "TKTJ", 4) == 0)   	// Kimi ni Todoke: Tsutaeru Kimochi (Japan)
	 || (memcmp(gameTid, "CKDJ", 4) == 0)   	// Kindaichi Shounen no Jikenbo: Akuma no Satsujin Koukai (Japan)
	 || (memcmp(gameTid, "VCGJ", 4) == 0)   	// Kirakira Rhythm Collection (Japan)
	 || (memcmp(gameTid, "BCKJ", 4) == 0)   	// Kochira Katsushika Ku Kameari Kouen Mae Hashutsujo: Kateba Tengoku! Makereba Jigoku!: Ryoutsu-ryuu Ikkakusenkin Daisakusen! (Japan)
	 || (memcmp(gameTid, "BCXJ", 4) == 0)   	// Kodawari Saihai Simulation: Ochanoma Pro Yakyuu DS: 2010 Nendo Ban (Japan)
	 || (memcmp(gameTid, "VKPJ", 4) == 0)   	// Korg DS-10+ Synthesizer Limited Edition (Japan)
	 || (memcmp(gameTid, "BZMJ", 4) == 0)   	// Korg M01 Music Workstation (Japan)
	 || (memcmp(gameTid, "BTAJ", 4) == 0)   	// Lina no Atelier: Strahl no Renkinjutsushi (Japan)
	 || (memcmp(gameTid, "BCDK", 4) == 0)   	// Live-On Card Live-R DS (Korea)
	 || (memcmp(gameTid, "VLIP", 4) == 0)   	// Lost Identities (Europe)
	 || (memcmp(gameTid, "BOXJ", 4) == 0)   	// Love Plus+ (Japan)
	 || (memcmp(gameTid, "BL3J", 4) == 0)   	// Lupin Sansei: Shijou Saidai no Zunousen (Japan)
	 || (memcmp(gameTid, "YNOK", 4) == 0)   	// Mabeopcheonjamun DS (Korea)
	 || (memcmp(gameTid, "BCJK", 4) == 0)   	// Mabeopcheonjamun DS 2 (Korea)
	 || (memcmp(gameTid, "ANMK", 4) == 0)   	// Maeilmaeil Deoukdeo!: DS Dunoe Training (Korea)
	 || (memcmp(gameTid, "TCYE", 4) == 0)   	// Mama's Combo Pack: Volume 1 (USA)
	 || (memcmp(gameTid, "TCZE", 4) == 0)   	// Mama's Combo Pack: Volume 2 (USA)
	 || (memcmp(gameTid, "ANMK", 4) == 0)   	// Marie-Antoinette and the American War of Independence: Episode 1: The Brotherhood of the Wolf (Europe)
	 || (memcmp(gameTid, "BA5K", 4) == 0)   	// Mario & Luigi RPG: Siganui Partner (Korea)
	 || (memcmp(gameTid, "C6OJ", 4) == 0)   	// Medarot DS: Kabuto Ver. (Japan)
	 || (memcmp(gameTid, "BQWJ", 4) == 0)   	// Medarot DS: Kuwagata Ver. (Japan)
	 || (memcmp(gameTid, "BBJJ", 4) == 0)   	// Metal Fight Beyblade: Baku Shin Susanoo Shuurai! (Japan)
	 || (memcmp(gameTid, "TKNJ", 4) == 0)   	// Meitantei Conan: Aoki Houseki no Rondo (Japan)
	 || (memcmp(gameTid, "TMKJ", 4) == 0)   	// Meitantei Conan: Kako Kara no Zensou Kyoku (Japan)
	 || (memcmp(gameTid, "TMXJ", 4) == 0)   	// Metal Max 2: Reloaded (Japan)
	 || (memcmp(gameTid, "C34J", 4) == 0)   	// Mini Yonku DS (Japan)
	 || (memcmp(gameTid, "BWCJ", 4) == 0)   	// Minna no Conveni (Japan)
	 || (memcmp(gameTid, "BQUJ", 4) == 0)   	// Minna no Suizokukan (Japan)
	 || (memcmp(gameTid, "BQVJ", 4) == 0)   	// Minna to Kimi no Piramekino! (Japan)
	 || (memcmp(gameTid, "B2WJ", 4) == 0)   	// Moe Moe 2-ji Taisen(ryaku) Two: Yamato Nadeshiko (Japan)
	 || (memcmp(gameTid, "BWRJ", 4) == 0)   	// Momotarou Dentetsu: World (Japan)
	 || (memcmp(gameTid, "CZZK", 4) == 0)   	// Monmallineun 3-gongjuwa Hamkkehaneun: Geurimyeonsang Yeongdaneo Amgibeop (Korea)
	 || (memcmp(gameTid, "B3IJ", 4) == 0)   	// Motto! Stitch! DS: Rhythm de Rakugaki Daisakusen (Japan)
	 || (memcmp(gameTid, "C6FJ", 4) == 0)   	// Mugen no Frontier Exceed: Super Robot Taisen OG Saga (Japan)
	 || (memcmp(gameTid, "B74J", 4) == 0)   	// Nanashi no Geemu Me (Japan)
	 || (memcmp(gameTid, "TNRJ", 4) == 0)   	// Nora to Toki no Koubou: Kiri no Mori no Majo (Japan)
	 || (memcmp(gameTid, "YNRK", 4) == 0)   	// Naruto Jilpungjeon: Daenantu! Geurimja Bunsinsul (Korea)
	 || (memcmp(gameTid, "BKJJ", 4) == 0)   	// Nazotte Oboeru: Otona no Kanji Renshuu: Kaiteiban (Japan)
	 || (memcmp(gameTid, "BPUJ", 4) == 0)   	// Nettou! Powerful Koushien (Japan)
	 || (memcmp(gameTid, "TJ7J", 4) == 0)   	// New Horizon: English Course 3 (Japan)
	 || (memcmp(gameTid, "TJ8J", 4) == 0)   	// New Horizon: English Course 2 (Japan)
	 || (memcmp(gameTid, "TJ9J", 4) == 0)   	// New Horizon: English Course 1 (Japan)
	 || (memcmp(gameTid, "BETJ", 4) == 0)   	// Nihon Keizai Shinbunsha Kanshuu: Shiranai Mama dewa Son wo Suru: 'Mono ya Okane no Shikumi' DS (Japan)
	 || (memcmp(gameTid, "YCUP", 4) == 0)   	// Nintendo Presents: Crossword Collection (Europe)
	 || (memcmp(gameTid, "B2KJ", 4) == 0)   	// Ni no Kuni: Shikkoku no Madoushi (Japan)
	 || (memcmp(gameTid, "BNCJ", 4) == 0)   	// Nodame Cantabile: Tanoshii Ongaku no Jikan Desu (Japan)
	 || (memcmp(gameTid, "CQKP", 4) == 0)   	// NRL Mascot Mania (Australia)
	 || (memcmp(gameTid, "BO4J", 4) == 0)   	// Ochaken no Heya DS 4 (Japan)
	 || (memcmp(gameTid, "BOYJ", 4) == 0)   	// Odoru Daisousa-sen: The Game: Sensuikan ni Sennyuu Seyo! (Japan)
	 || (memcmp(gameTid, "B62J", 4) == 0)   	// Okaeri! Chibi-Robo!: Happy Rich Oosouji! (Japan)
	 || (memcmp(gameTid, "TGBJ", 4) == 0)   	// One Piece Gigant Battle 2: Shin Sekai (Japan)
	 || (memcmp(gameTid, "BOKJ", 4) == 0)   	// Ookami to Koushinryou: Umi o Wataru Kaze (Japan)
	 || (memcmp(gameTid, "TKDJ", 4) == 0)   	// Ore-Sama Kingdom: Koi mo Manga mo Debut o Mezase! Doki Doki Love Lesson (Japan)
	 || (memcmp(gameTid, "TFTJ", 4) == 0)   	// Original Story from Fairy Tail: Gekitotsu! Kardia Daiseidou (Japan)
	 || (memcmp(gameTid, "BHQJ", 4) == 0)   	// Otona no Renai Shousetsu: DS Harlequin Selection (Japan)
	 || (memcmp(gameTid, "BIPJ", 4) == 0)   	// Pen1 Grand Prix: Penguin no Mondai Special (Japan)
	 || (memcmp(gameTid, "BO9J", 4) == 0)   	// Penguin no Mondai: The World (Japan)
	 || (memcmp(gameTid, "B42J", 4) == 0)   	// Pet Shop Monogatari DS 2 (Japan)
	 || (memcmp(gameTid, "BVGE", 4) == 0)   	// Petz: Bunnyz Bunch (USA)
	 || (memcmp(gameTid, "BLLE", 4) == 0)   	// Petz: Catz Playground (USA)
	 || (memcmp(gameTid, "BUFE", 4) == 0)   	// Petz: Puppyz & Kittenz (USA)
	 || (memcmp(gameTid, "VFBE", 4) == 0)   	// Petz Fantasy: Moonlight Magic (USA)
	 || (memcmp(gameTid, "VTPV", 4) == 0)   	// Phineas and Ferb: 2 Disney Games (Europe)
	 || (memcmp(gameTid, "B5VE", 4) == 0)   	// Phineas and Ferb: Across the 2nd Dimension (USA)
	 || (memcmp(gameTid, "YFTK", 4) == 0)   	// Pokemon Bulgasaui Dungeon: Siganui Tamheomdae (Korea)
	 || (memcmp(gameTid, "YFYK", 4) == 0)   	// Pokemon Bulgasaui Dungeon: Eodumui Tamheomdae (Korea)
	 || (memcmp(gameTid, "BPPJ", 4) == 0)   	// PostPet DS: Yumemiru Momo to Fushigi no Pen (Japan)
	 || (memcmp(gameTid, "BONJ", 4) == 0)   	// Powerful Golf (Japan)
	 || (memcmp(gameTid, "VPTJ", 4) == 0)   	// Power Pro Kun Pocket 12 (Japan)
	 || (memcmp(gameTid, "VPLJ", 4) == 0)   	// Power Pro Kun Pocket 13 (Japan)
	 || (memcmp(gameTid, "VP4J", 4) == 0)   	// Power Pro Kun Pocket 14 (Japan)
	 || (memcmp(gameTid, "B2YK", 4) == 0)   	// Ppiyodamari DS (Korea)
	 || (memcmp(gameTid, "B4NK", 4) == 0)   	// Princess Angel: Baeguiui Cheonsa (Korea)
	 || (memcmp(gameTid, "C4WK", 4) == 0)   	// Princess Bakery (Korea)
	 || (memcmp(gameTid, "CP4K", 4) == 0)   	// Princess Maker 4: Special Edition (Korea)
	 || (memcmp(gameTid, "C29J", 4) == 0)   	// Pro Yakyuu Famista DS 2009 (Japan)
	 || (memcmp(gameTid, "BF2J", 4) == 0)   	// Pro Yakyuu Famista DS 2010 (Japan)
	 || (memcmp(gameTid, "B89J", 4) == 0)   	// Pro Yakyuu Team o Tsukurou! 2 (Japan)
	 || (memcmp(gameTid, "BU9J", 4) == 0)   	// Pucca: Power Up (Europe)
	 || (memcmp(gameTid, "TP4J", 4) == 0)   	// Puyo Puyo!!: Puyopuyo 20th Anniversary (Japan)
	 || (memcmp(gameTid, "BYOJ", 4) == 0)   	// Puyo Puyo 7 (Japan)
	 || (memcmp(gameTid, "BHXJ", 4) == 0)   	// Quiz! Hexagon II (Japan)
	 || (memcmp(gameTid, "BQ2J", 4) == 0)   	// Quiz Magic Academy DS: Futatsu no Jikuuseki (Japan)
	 || (memcmp(gameTid, "YRBK", 4) == 0)   	// Ragnarok DS (Korea)
	 || (memcmp(gameTid, "TEDJ", 4) == 0)   	// Red Stone DS: Akaki Ishi ni Michibikareshi Mono-tachi (Japan)
	 || (memcmp(gameTid, "B35J", 4) == 0)   	// Rekishi Simulation Game: Sangokushi DS 3 (Japan)
	 || (memcmp(gameTid, "BUKJ", 4) == 0)   	// Rekishi Taisen: Gettenka: Tenkaichi Battle Royale (Japan)
	 || (memcmp(gameTid, "YLZK", 4) == 0)   	// Rhythm Sesang (Korea)
	 || (memcmp(gameTid, "BKMJ", 4) == 0)   	// Rilakkuma Rhythm: Mattari Kibun de Dararan Ran (Japan)
	 || (memcmp(gameTid, "B6XJ", 4) == 0)   	// Rockman EXE: Operate Shooting Star (Japan)
	 || (memcmp(gameTid, "V29J", 4) == 0)   	// RPG Tkool DS (Japan)
	 || (memcmp(gameTid, "VEBJ", 4) == 0)   	// RPG Tsukuru DS+: Create The New World (Japan)
	 || (memcmp(gameTid, "ARFK", 4) == 0)   	// Rune Factory: Sinmokjjangiyagi (Korea)
	// || (memcmp(gameTid, "CSGJ", 4) == 0)   	// SaGa 2: Hihou Densetsu: Goddess of Destiny (Japan) (Patched by nds-bootstrap)
	 || (memcmp(gameTid, "BZ3J", 4) == 0)   	// SaGa 3: Jikuu no Hasha: Shadow or Light (Japan)
	 || (memcmp(gameTid, "CBEJ", 4) == 0)   	// Saibanin Suiri Game: Yuuzai x Muzai (Japan)
	 || (memcmp(gameTid, "B59J", 4) == 0)   	// Sakusaku Jinkou Kokyuu Care Training DS (Japan)
	 || (memcmp(gameTid, "BSWJ", 4) == 0)   	// Saka Tsuku DS: World Challenge 2010 (Japan)
	 || (memcmp(gameTid, "B3GJ", 4) == 0)   	// SD Gundam Sangoku Den: Brave Battle Warriors: Shin Militia Taisen (Japan)
	 || (memcmp(gameTid, "B7XJ", 4) == 0)   	// Seitokai no Ichizon: DS Suru Seitokai (Japan)
	 || (memcmp(gameTid, "CQ2J", 4) == 0)   	// Sengoku Spirits: Gunshi Den (Japan)
	 || (memcmp(gameTid, "CQ3J", 4) == 0)   	// Sengoku Spirits: Moushou Den (Japan)
	 || (memcmp(gameTid, "YR4J", 4) == 0)   	// Sengoku Spirits: Shukun Den (Japan)
	 || (memcmp(gameTid, "B5GJ", 4) == 0)   	// Shin Sengoku Tenka Touitsu: Gunyuu-tachi no Souran (Japan)
	 || (memcmp(gameTid, "C36J", 4) == 0)   	// Sloane to MacHale no Nazo no Story (Japan)
	 || (memcmp(gameTid, "B2QJ", 4) == 0)   	// Sloane to MacHale no Nazo no Story 2 (Japan)
	 || (memcmp(gameTid, "A3YK", 4) == 0)   	// Sonic Rush Adventure (Korea)
	 || (memcmp(gameTid, "TFLJ", 4) == 0)   	// Sora no Otoshimono Forte: Dreamy Season (Japan)
	 || (memcmp(gameTid, "YW4K", 4) == 0)   	// Spectral Force: Genesis (Korea)
	 || (memcmp(gameTid, "B22J", 4) == 0)   	// Strike Witches 2: Iyasu, Naosu, Punipuni Suru (Japan)
	 || (memcmp(gameTid, "BYQJ", 4) == 0)   	// Suisui Physical Assessment Training DS (Japan)
	 || (memcmp(gameTid, "TPQJ", 4) == 0)   	// Suite PreCure: Melody Collection (Japan)
	 || (memcmp(gameTid, "CS7J", 4) == 0)   	// Summon Night X: Tears Crown (Japan)
	 || (memcmp(gameTid, "C2YJ", 4) == 0)   	// Supa Robo Gakuen (Japan) Nazotoki Adventure (Japan)
	 || (memcmp(gameTid, "BRWJ", 4) == 0)   	// Super Robot Taisen L (Japan)
	 || (memcmp(gameTid, "BROJ", 4) == 0)   	// Super Robot Taisen OG Saga: Masou Kishin: The Lord of Elemental (Japan)
	 || (memcmp(gameTid, "C5IJ", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-1-shuu: Nazotoki Sekai Isshuu Ryokou (Japan)
	 || (memcmp(gameTid, "C52J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-2-shuu: Ginga Oudan (Japan)
	 || (memcmp(gameTid, "BQ3J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-3-shuu: Fushigi no Kuni no Nazotoki Otogibanashi (Japan)
	 || (memcmp(gameTid, "BQ4J", 4) == 0)   	// Tago Akira no Atama no Taisou: Dai-4-shuu: Time Machine no Nazotoki Daibouken (Japan)
	 || (memcmp(gameTid, "B3DJ", 4) == 0)   	// Taiko no Tatsujin DS: Dororon! Yookai Daikessen!! (Japan)
	 || (memcmp(gameTid, "B7KJ", 4) == 0)   	// Tamagotch no Narikiri Challenge (Japan)
	 || (memcmp(gameTid, "BGVJ", 4) == 0)   	// Tamagotch no Narikiri Channel (Japan)
	 || (memcmp(gameTid, "BG5J", 4) == 0)   	// Tamagotch no Pichi Pichi Omisetchi (Japan)
	 || (memcmp(gameTid, "TGCJ", 4) == 0)   	// Tamagotchi Collection (Japan)
	 || (memcmp(gameTid, "BQ9J", 4) == 0)   	// Tekipaki Kyuukyuu Kyuuhen Training DS (Japan)
	 || (memcmp(gameTid, "B5KJ", 4) == 0)   	// Tenkaichi: Sengoku Lovers DS (Japan)
	 || (memcmp(gameTid, "TENJ", 4) == 0)   	// Tennis no Ouji-sama: Gyutto! Dokidoki Survival: Umi to Yama no Love Passion (Japan)
	 || (memcmp(gameTid, "BTGJ", 4) == 0)   	// Tennis no Ouji-sama: Motto Gakuensai no Ouji-sama: More Sweet Edition (Japan)
	 || (memcmp(gameTid, "VIMJ", 4) == 0)   	// The Idolm@ster: Dearly Stars (Japan)
	 || (memcmp(gameTid, "B6KP", 4) == 0)   	// Tinker Bell + Tinker Bell and the Lost Treasure (Europe)
	 || (memcmp(gameTid, "TKGJ", 4) == 0)   	// Tobidase! Kagaku-kun: Chikyuu Daitanken! Nazo no Chinkai Seibutsu ni Idome! (Japan)
	 || (memcmp(gameTid, "CT5K", 4) == 0)   	// TOEIC DS: Haru 10-bun Yakjeomgeukbok +200 (Korea)
	 || (memcmp(gameTid, "AEYK", 4) == 0)   	// TOEIC Test DS Training (Korea)
	 || (memcmp(gameTid, "BT5J", 4) == 0)   	// TOEIC Test Super Coach@DS (Japan)
	 || (memcmp(gameTid, "TQ5J", 4) == 0)   	// Tokumei Sentai Go Busters (Japan)
	 || (memcmp(gameTid, "CVAJ", 4) == 0)   	// Tokyo Twilight Busters: Kindan no Ikenie Teito Jigokuhen (Japan)
	 || (memcmp(gameTid, "CZXK", 4) == 0)   	// Touch Man to Man: Gichoyeongeo (Korea)
	 || (memcmp(gameTid, "BUQJ", 4) == 0)   	// Treasure Report: Kikai Jikake no Isan (Japan)
	 || (memcmp(gameTid, "C2VJ", 4) == 0)   	// Tsukibito (Japan)
	 || (memcmp(gameTid, "BH6J", 4) == 0)   	// TV Anime Fairy Tail: Gekitou! Madoushi Kessen (Japan)
	 || (memcmp(gameTid, "CUHJ", 4) == 0)   	// Umihara Kawase Shun: Second Edition Kanzen Ban (Japan)
	 || (memcmp(gameTid, "TBCJ", 4) == 0)   	// Usavich: Game no Jikan (Japan)
	 || (memcmp(gameTid, "BPOJ", 4) == 0)   	// Utacchi (Japan)
	 || (memcmp(gameTid, "BXPJ", 4) == 0)   	// Winnie the Pooh: Kuma no Puu-san: 100 Acre no Mori no Cooking Book (Japan)
	 || (memcmp(gameTid, "BWYJ", 4) == 0)   	// Wizardry: Boukyaku no Isan (Japan)
	 || (memcmp(gameTid, "BWZJ", 4) == 0)   	// Wizardry: Inochi no Kusabi (Japan)
	 || (memcmp(gameTid, "BWWJ", 4) == 0)   	// WiZmans World (Japan)
	 || (memcmp(gameTid, "BYNJ", 4) == 0)   	// Yamakawa Shuppansha Kanshuu: Shousetsu Nihonshi B: Shin Sougou Training Plus (Japan)
	 || (memcmp(gameTid, "BYSJ", 4) == 0)   	// Yamakawa Shuppansha Kanshuu: Shousetsu Sekaishi B: Shin Sougou Training Plus (Japan)
	 || (memcmp(gameTid, "B5DJ", 4) == 0)   	// Yamanote-sen Meimei 100 Shuunen Kinen: Densha de Go!: Tokubetsu Hen: Fukkatsu! Shouwa no Yamanote-sen (Japan)
	 || (memcmp(gameTid, "BYMJ", 4) == 0)   	// Yumeiro Patissiere: My Sweets Cooking (Japan)
	 || (memcmp(gameTid, "BZQJ", 4) == 0)   	// Zac to Ombra: Maboroshi no Yuuenchi (Japan)
	 || (memcmp(gameTid, "BZBJ", 4) == 0)) {	// Zombie Daisuki (Japan)
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
			if (memcmp(gameTid, ap_list[i], 3) == 0) {
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
			if (memcmp(gameTid, ap_list2[i], 3) == 0) {
				// Found a match.
				return 2;
				break;
			}
		}

	}
	
	return 0;
}

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
	if (ms().theme != TWLSettings::EThemeGBC || ms().filenameDisplay < 2) {
		preloadedBannerIcons = new sNDSBannerExt[count];
	}
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
static u16 bnriconframeseq[64] = {0x0000};

// bnriconframenum[]
int bnriconPalLoaded = 0;
int bnriconPalLine = 0;
int bnriconPalLinePrev = 0;
int bnriconframenumY = 0;
int bnriconframenumYPrev = 0;
int bannerFlip = GL_FLIP_NONE;
int bannerFlipPrev = GL_FLIP_NONE;

// bnriconisDSi[]
bool isValid = false;
bool isTwlm = false;
bool isDirectory = false;
int bnrRomType = 0;
bool bnriconisDSi = false;
int bnrWirelessIcon = 0; // 0 = None, 1 = Local, 2 = WiFi
char gameTid[5] = {0};
u8 romVersion = 0;
u8 romUnitCode = 0;
u32 a7mbk6 = 0;
bool isDSiWare = false;
bool isHomebrew = false;
bool isModernHomebrew = false;		// false == No DSi-Extended header, true == Has DSi-Extended header
bool requiresRamDisk = false;
int requiresDonorRom = 0;
int customIcon = 0;					// 0 = None, 1 = png, 2 = banner.bin, -1 = error
char customIconPath[256];

static u16 bannerDelayNum = 0x0000;
int currentbnriconframeseq = 0;

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence()
{
	memcpy(bnriconframeseq, ndsBanner.dsi_seq, 64 * sizeof(u16));
	currentbnriconframeseq = 0;
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence()
{
	memset(bnriconframeseq, 0, 64 * sizeof(u16));
	currentbnriconframeseq = 0;
}

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
bool playBannerSequence()
{
	if (bnriconframeseq[currentbnriconframeseq + 1] == 0x0100) {
		// Do nothing if icon isn't animated
		bnriconPalLine = 0;
		bnriconframenumY = 0;
		bannerFlip = GL_FLIP_NONE;
	} else {
		u16 setframeseq = bnriconframeseq[currentbnriconframeseq];
		bnriconPalLine = SEQ_PAL(setframeseq);
		bnriconframenumY =  SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV) {
			bannerFlip = GL_FLIP_H | GL_FLIP_V;
		} else if (!flipH && !flipV) {
			bannerFlip = GL_FLIP_NONE;
		} else if (flipH && !flipV) {
			bannerFlip = GL_FLIP_H;
		} else if (!flipH && flipV) {
			bannerFlip = GL_FLIP_V;
		}

		bool updateIcon = false;

		if (bnriconPalLinePrev != bnriconPalLine) {
			bnriconPalLinePrev = bnriconPalLine;
			updateIcon = true;
		}

		if (bnriconframenumYPrev != bnriconframenumY) {
			bnriconframenumYPrev = bnriconframenumY;
			updateIcon = true;
		}

		if (bannerFlipPrev != bannerFlip) {
			bannerFlipPrev = bannerFlip;
			updateIcon = true;
		}

		bannerDelayNum++;
		if (bannerDelayNum >= (setframeseq & 0x00FF)) {
			bannerDelayNum = 0x0000;
			currentbnriconframeseq++;
			if (bnriconframeseq[currentbnriconframeseq] == 0x0000) {
				currentbnriconframeseq = 0; // Reset sequence
			}
		}

		return updateIcon;
	}

	return false;
}
