#include <nds.h>
#include <stdio.h>

#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "common/tonccpy.h"
#include "common/inifile.h"
#include "tool/stringtool.h"
#include "graphics/fontHandler.h"
#include "graphics/ThemeTextures.h"
#include "graphics/lodepng.h"
#include "ndsheaderbanner.h"
#include "language.h"

#include "dsGameInfoMap.h"

void createEsrbSplash(void) {
	if (isHomebrew[CURPOS] || (gameTid[CURPOS][3] != 'E' && gameTid[CURPOS][3] != 'O' && gameTid[CURPOS][3] != 'T')) {
		remove(sdFound() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");
		return;
	}

	char gameTid3[4] = {0};
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = gameTid[CURPOS][i];
	}

	CIniFile esrbInfo( "nitro:/ESRB.ini" );
	std::string rating = esrbInfo.GetString(gameTid3, "Rating", "");
	if (rating == "") {
		remove(sdFound() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin");
		return;
	}

	std::string descriptors = esrbInfo.GetString(gameTid3, "Descriptors en", "");

	char esrbImagePath[64];
	if (rating == "E" && descriptors == "") {
		// Search for games starting sideways
		bool sideways = false;
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(sidewaysGameList)/sizeof(sidewaysGameList[0]); i++) {
			if (memcmp(gameTid[CURPOS], sidewaysGameList[i], 3) == 0) {
				// Found match
				sideways = true;
				break;
			}
		}
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/E-%s.png", sideways ? "side" : "nodesc"); 
	} else {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/%s.png", rating.c_str()); 
	}

	DC_FlushAll();

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, esrbImagePath);
	if(imageWidth > 256 || imageHeight > 192) return;

	for(uint i=0;i<image.size()/4;i++) {
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
				descriptorYpos -= 6;
				currentChar = 0;
				if (descriptors[i+1] == ' ') {
					i++;
				}
			} else {
				descriptorList[descriptorLines-1][currentChar] = descriptors[i];
				currentChar++;
			}
		}

		clearText(true);
		for (int i = 0; i < descriptorLines; i++) {
			printSmall(true, 100, descriptorYpos, descriptorList[i]);
			descriptorYpos += 12;
		}
		updateTextImg(tex().bmpImageBuffer(), true);
		clearText(true);
	}

	mkdir(sdFound() ? "sd:/_nds/nds-bootstrap" : "fat:/_nds/nds-bootstrap", 0777);

	FILE *file = fopen(sdFound() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin", "wb");
	fwrite(tex().bmpImageBuffer(), sizeof(u16), 256*192, file);
	fclose(file);
}