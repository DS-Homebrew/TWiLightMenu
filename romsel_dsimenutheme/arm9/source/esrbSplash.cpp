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

	char esrbImagePath[64];
	if (rating == "E") {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/E-nodesc.png"); 
	} else {
		sprintf(esrbImagePath, "nitro:/graphics/ESRB/%s.png", rating.c_str()); 
	}

	std::vector<unsigned char> image;
	uint imageWidth, imageHeight;
	lodepng::decode(image, imageWidth, imageHeight, esrbImagePath);
	if(imageWidth > 256 || imageHeight > 192) return;

	for(uint i=0;i<image.size()/4;i++) {
		tex().bmpImageBuffer()[i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
	}

	// TODO: Write the rating descriptor to the blank space

	mkdir(sdFound() ? "sd:/_nds/nds-bootstrap" : "fat:/_nds/nds-bootstrap", 0777);

	FILE *file = fopen(sdFound() ? "sd:/_nds/nds-bootstrap/esrb.bin" : "fat:/_nds/nds-bootstrap/esrb.bin", "wb");
	fwrite(tex().bmpImageBuffer(), sizeof(u16), 256*192, file);
	fclose(file);
}