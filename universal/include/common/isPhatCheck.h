#ifndef IS_PHAT_CHECK_H
#define IS_PHAT_CHECK_H

#include <string.h>

#define FLASHM_BACKUPHEADER 0x3f680
#define FLASHME_MAJOR_VER 0x17c

static inline bool isPhat() {
	if(!!(REG_SNDEXTCNT)) return false;
	u8 consoleType = 0;
	readFirmware(0x1D, &consoleType, 1);
	if(consoleType == 0xff) return true;
	
	u8 flashmeVersion = 0;
	readFirmware(FLASHME_MAJOR_VER, &flashmeVersion, 1);
	
	if(flashmeVersion != 0xff){
		u8 contentsOnLite[6] = {0xff,0xff,0xff,0xff,0xff,0x00};
		u8 unusedShouldBeZeroFilledOnPhat[6];
		readFirmware(FLASHM_BACKUPHEADER+0x30, &unusedShouldBeZeroFilledOnPhat, 6);
		return memcmp(unusedShouldBeZeroFilledOnPhat, contentsOnLite,6) != 0;
	}
	return false;
}

#endif //IS_PHAT_CHECK_H