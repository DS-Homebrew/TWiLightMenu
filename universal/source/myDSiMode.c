#include <nds/ndstypes.h>
#include <nds/interrupts.h>
#include <nds/system.h>

#ifdef ARM9
static bool twlSpeedChecked = false;
static bool twlSpeed = false;

ITCM_CODE bool isTwlSpeed(void) {
	if (!twlSpeedChecked) {
		if (REG_SCFG_EXT != 0) {
			twlSpeed = REG_SCFG_CLK & BIT(0);
		} else {
			int oldIME = enterCriticalSection();
			int i = 0;

			while (REG_VCOUNT != 191);
			while (REG_VCOUNT == 191);

			while (REG_VCOUNT != 191) {
				i++;
			}

			leaveCriticalSection(oldIME);

			twlSpeed = (i >= 100000 && i < 150000);
		}
		twlSpeedChecked = true;
	}
	return twlSpeed;
}
#endif