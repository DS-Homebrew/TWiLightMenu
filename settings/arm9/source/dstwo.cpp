// SuperCard DSTWO: unlock the SD card interface in Slot-1.
//
// The SD card is only reachable through the DSTWO's MIPS CPU, which boots when the DS replays the
// boot protocol of the DSTWO's own stub. The DSTWO DLDI's startup() loops forever until the
// handshake (0xE1, then 0x73 answered by 0xA5A55A5A) works, so it has to work before the DLDI is loaded.

#include <nds.h>

#include "dstwo.h"
#include "read_card.h"

#define DSTWO_ROMCTRL   (*(vu32*)0x040001A4)
#define DSTWO_AUXSPICNT_H (*(vu8*)0x040001A1)
#define DSTWO_CMD       ((vu8*)0x040001A8)
#define DSTWO_DATA      (*(vu32*)0x04100010)

#define DSTWO_MAGIC_ARG  0x5A5AA5A5
#define DSTWO_MAGIC_RESP 0xA5A55A5A
#define DSTWO_TIMEOUT    0x200000

// Time is counted in frames: vblank waits count, and a timed out command is charged
// DSTWO_TIMEOUT_FRAMES. A working DSTWO is ready after about 1.5 seconds.
#define DSTWO_UNLOCK_BUDGET_FRAMES 480
#define DSTWO_READY_WAIT_FRAMES 240
#define DSTWO_RETRY_MAX_USED_FRAMES 330
#define DSTWO_TIMEOUT_FRAMES 8

static int dstwoFramesUsed;
static bool dstwoUnlockFailed;

static bool dstwoExpired(void) {
	return dstwoFramesUsed >= DSTWO_UNLOCK_BUDGET_FRAMES;
}

// Sends one 8-byte command and returns the first reply word in *reply (0xFFFFFFFF if none).
// Returns false on timeout.
static bool dstwoCommand(u8 op, u32 arg1, u32 arg2, u32 romctrl, u32* reply) {
	int t = DSTWO_TIMEOUT;
	while ((DSTWO_ROMCTRL & 0x80000000) && --t);
	if (!t) {
		dstwoFramesUsed += DSTWO_TIMEOUT_FRAMES;
		*reply = 0xFFFFFFFF;
		return false;
	}

	DSTWO_AUXSPICNT_H = 0x80;
	DSTWO_CMD[0] = op;
	DSTWO_CMD[1] = arg1 >> 24; DSTWO_CMD[2] = arg1 >> 16; DSTWO_CMD[3] = arg1 >> 8; DSTWO_CMD[4] = arg1;
	DSTWO_CMD[5] = arg2 >> 16; DSTWO_CMD[6] = arg2 >> 8;  DSTWO_CMD[7] = arg2;
	DSTWO_ROMCTRL = romctrl;

	bool gotWord = false;
	u32 first = 0xFFFFFFFF;
	t = DSTWO_TIMEOUT;
	while (--t) {
		const u32 ctrl = DSTWO_ROMCTRL;
		if (ctrl & 0x00800000) {
			const u32 word = DSTWO_DATA;
			if (!gotWord) {
				first = word;
				gotWord = true;
			}
		}
		if (!(ctrl & 0x80000000)) break;
	}
	if (!t) dstwoFramesUsed += DSTWO_TIMEOUT_FRAMES;
	*reply = first;
	return t != 0;
}

static void dstwoWaitFrames(int frames) {
	for (int i = 0; i < frames; i++) swiWaitForVBlank();
	dstwoFramesUsed += frames;
}

// The stub's 0x16 handshake: the card answers with the inverted argument.
static bool dstwoSync(int tries) {
	u32 reply = 0;
	for (int i = 0; i < tries && !dstwoExpired(); i++) {
		dstwoCommand(0x16, 0x12345678, 0, 0xA7180910, &reply);
		if (~reply == 0x12345678) return true;
		dstwoWaitFrames(4);
	}
	return false;
}

static bool dstwoDldiHandshake(int frames, bool sendE1) {
	for (int i = 0; i < frames && !dstwoExpired(); i++) {
		u32 reply = 0;
		if (sendE1) dstwoCommand(0xE1, 0, 0, 0xA7180000, &reply);
		if (dstwoCommand(0x73, DSTWO_MAGIC_ARG, 0, 0xA7180018, &reply) && reply == DSTWO_MAGIC_RESP) {
			return true;
		}
		dstwoWaitFrames(1);
	}
	return false;
}

static bool dstwoBoot(void) {
	u32 reply = 0, status = 0;

	if (!dstwoCommand(0x15, 0, 0, 0xA7586090, &reply)) return false;
	if (reply == 0x50505050) { // 'PPPP': MIPS not ready yet
		dstwoSync(10);
		dstwoCommand(0x15, 0, 0, 0xA7180090, &reply);
	}

	// Wait for the MIPS to report the SD card (status & 7 == 7). A missing reply reads as
	// 0xFFFFFFFF and must not count.
	bool ready = false;
	const int waitStart = dstwoFramesUsed;
	while (!ready && !dstwoExpired() && dstwoFramesUsed - waitStart < DSTWO_READY_WAIT_FRAMES) {
		const bool synced = dstwoSync(5);
		if (synced && dstwoCommand(0x17, 0, 0, 0xA7180910, &status)
		 && status != 0xFFFFFFFF && (status & 7) == 7) {
			ready = true;
		} else {
			dstwoWaitFrames(6);
		}
	}
	if (!ready) return false;

	dstwoCommand(0x15, 0, 0, 0xA7180090, &reply);
	dstwoCommand(0xE0, 0, 0, 0xA7180010, &reply);

	return dstwoDldiHandshake(30, true) || dstwoDldiHandshake(30, false);
}

// Same card initialisation as twl_flashcardInit(): ARM7 first, then a header read from the ARM9.
static bool dstwoCardInit(void) {
	static u8 header[0x200] __attribute__((aligned(4)));

	sysSetCardOwner(BUS_OWNER_ARM7);
	if (!fifoSendValue32(FIFO_USER_02, 0x54494E49)) { // 'INIT'
		sysSetCardOwner(BUS_OWNER_ARM9);
		return false;
	}
	int waited = 0;
	while (!fifoCheckValue32(FIFO_USER_02) && waited < 300) {
		swiWaitForVBlank();
		waited++;
	}
	dstwoFramesUsed += waited;
	if (!fifoCheckValue32(FIFO_USER_02)) {
		sysSetCardOwner(BUS_OWNER_ARM9);
		return false;
	}
	const u32 result = fifoGetValue32(FIFO_USER_02);
	sysSetCardOwner(BUS_OWNER_ARM9);
	if (result != 0) return false;

	cardReadHeader(header);
	return true;
}

static bool dstwoUnlockInner(void) {
	sysSetCardOwner(BUS_OWNER_ARM9);

	// The card is normally still in the state left by twl_flashcardInit(). Otherwise power
	// cycle the slot, initialise it again and retry once.
	if (dstwoBoot()) return true;
	if (dstwoExpired() || dstwoFramesUsed > DSTWO_RETRY_MAX_USED_FRAMES) return false;

	disableSlot1();
	dstwoWaitFrames(60);
	if (REG_SCFG_MC == 0x11) return false;
	enableSlot1();
	dstwoWaitFrames(30);
	if (!dstwoCardInit()) return false;
	dstwoWaitFrames(30);
	return !dstwoExpired() && dstwoBoot();
}

TWL_CODE bool dstwoUnlock(void) {
	if (dstwoUnlockFailed) return false;

	dstwoFramesUsed = 0;
	if (dstwoUnlockInner()) return true;

	dstwoUnlockFailed = true;
	return false;
}
