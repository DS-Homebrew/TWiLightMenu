#ifndef MY_DSI_MODE_INCLUDE
#define MY_DSI_MODE_INCLUDE

#include <nds/ndstypes.h>
#include <nds/system.h>

#ifdef __cplusplus
extern "C" {
#endif

// Checks if DSi features are enabled,
// regardless if the console runs in DS or DSi mode
static inline 
bool dsiFeatures(void) {
	return (isDSiMode() || REG_SCFG_EXT != 0);
}

#ifdef ARM9
// Checks for TWL clock speed without SCFG access
extern bool isTwlSpeed(void);
#endif

#ifdef __cplusplus
}
#endif
#endif