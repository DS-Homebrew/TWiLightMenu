#include <nds.h>
#include <stdio.h>

#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "common/tonccpy.h"
#include "common/inifile.h"
#include "common/logging.h"
#include "fileBrowse.h"
#include "graphics/fontHandler.h"
#include "graphics/ThemeTextures.h"
#include "common/lodepng.h"
#include "ndsheaderbanner.h"
#include "language.h"

#include "dsGameInfoMap.h"

void createEsrbSplash(void) {
	if (!ms().esrbRatingScreen || isHomebrew[CURPOS] || (gameTid[CURPOS][3] != 'E' && gameTid[CURPOS][3] != 'O' && gameTid[CURPOS][3] != 'T' && gameTid[CURPOS][3] != 'W')) {
		remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");
		return;
	}

	char gameTid3[4] = {0};
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = gameTid[CURPOS][i];
	}

	CIniFile esrbInfo("nitro:/ESRB.ini");
	std::string rating = esrbInfo.GetString(gameTid3, "Rating", "");
	if (rating == "") {
		remove(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");
		return;
	}

	std::string descriptors = esrbInfo.GetString(gameTid3, "Descriptors en", "");

	bool sideways = false;
	if ((rating == "E" || rating == "EC" || rating == "RP") && descriptors == "") {
		// Search for games starting sideways
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(sidewaysGameList)/sizeof(sidewaysGameList[0]); i++) {
			if (memcmp(gameTid[CURPOS], sidewaysGameList[i], 3) == 0) {
				// Found match
				sideways = true;
				break;
			}
		}
	}

	char esrbImagePath[64];
	if (rating == "E" && descriptors == "") {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/E-%s.png", sideways ? "side" : "nodesc"); 
	} else if (rating == "EC" || rating == "RP") {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/%s%s.png", rating.c_str(), sideways ? "-side" : ""); 
	} else {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/%s.png", rating.c_str()); 
	}
	logPrint("ESRB Rating: %s\n", rating.c_str());
	logPrint("ESRB Descriptors: %s\n", descriptors.c_str());

	DC_FlushAll();

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, esrbImagePath);
	if (imageWidth > 256 || imageHeight > 192) return;

	for (uint i=0;i<image.size()/4;i++) {
		tex().bmpImageBuffer()[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
	}

	if (descriptors != "") {
		int descriptorLines = 1;
		int descriptorYpos = 80;
		char descriptorList[6][32] = {{0}};

		int currentChar = 0;
		for (int i = 0; i < 256; i++) {
			if (descriptors[i] == 0) {
				break;
			} else if (descriptors[i] == ',') {
				descriptorLines++;
				currentChar = 0;
				if (descriptors[i+1] == ' ') {
					i++;
				}
			} else {
				descriptorList[descriptorLines-1][currentChar] = descriptors[i];
				currentChar++;
			}
		}

		bool dsFont = false;
		for (int i = 0; i < descriptorLines; i++) {
			dsFont = strlen(descriptorList[i]) > 21;
			if (dsFont) {
				descriptorYpos += 2;
				break;
			}
		}

		esrbDescFontInit(dsFont);

		descriptorYpos -= ((descriptorLines > 4) ? 6 : 8)*(descriptorLines-1);

		clearText();
		for (int i = 0; i < descriptorLines; i++) {
			printSmall(false, 100, descriptorYpos, descriptorList[i]);
			descriptorYpos += (descriptorLines > 4) ? 12 : 16;
		}
		updateTextImg(tex().bmpImageBuffer(), false);
		clearText();

		esrbDescFontDeinit();
	}

	mkdir(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap" : "fat:/_nds/nds-bootstrap", 0777);

	FILE *file = fopen(sys().isRunFromSD() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin", "wb");
	fwrite(tex().bmpImageBuffer(), sizeof(u16), 256*192, file);
	fclose(file);
}
