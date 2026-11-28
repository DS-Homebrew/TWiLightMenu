#ifndef DSTWO_H
#define DSTWO_H

#include <nds/ndstypes.h>

// Brings up the SD interface of a SuperCard DSTWO in Slot-1 (DSi mode). Call it after the slot
// was power cycled and the card header read, as twl_flashcardInit() does. Returns true once the
// DLDI handshake works, i.e. once the DSTWO DLDI's startup() is guaranteed to return.
bool dstwoUnlock(void);

#endif // DSTWO_H
