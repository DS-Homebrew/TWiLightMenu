#include "i2c_handler.h"

#include "aptina.h"
#include "aptina_i2c.h"

#include <nds.h>

// Management structure and stack space for PXI server thread
Thread s_i2cPxiThread;
alignas(8) u8 s_i2cPxiThreadStack[1024];

//---------------------------------------------------------------------------------
int i2cPxiThreadMain(void* arg) {
//---------------------------------------------------------------------------------
	// Set up PXI mailbox, used to receive PXI command words
	Mailbox mb;
	u32 mb_slots[4];
	mailboxPrepare(&mb, mb_slots, sizeof(mb_slots)/4);
	pxiSetMailbox(PxiChannel_User0, &mb);

	// Main PXI message loop
	for (;;) {
		// Receive a message
		u32 msg = mailboxRecv(&mb);
		u32 retval = 0;

		switch (msg) {
			case CAM_INIT:
				init(I2C_CAM0);
				init(I2C_CAM1);
				retval = aptReadRegister(I2C_CAM0, 0);
				break;
			case CAM0_ACTIVATE:
				activate(I2C_CAM0);
				retval = CAM0_ACTIVATE;
				break;
			case CAM0_DEACTIVATE:
				deactivate(I2C_CAM0);
				retval = CAM0_DEACTIVATE;
				break;
			case CAM1_ACTIVATE:
				activate(I2C_CAM1);
				retval = CAM1_ACTIVATE;
				break;
			case CAM1_DEACTIVATE:
				deactivate(I2C_CAM1);
				retval = CAM1_DEACTIVATE;
				break;
			case CAM_SET_MODE_PREVIEW:
				setMode(CAPTURE_MODE_PREVIEW);
				retval = CAPTURE_MODE_PREVIEW;
				break;
			case CAM_SET_MODE_CAPTURE:
				setMode(CAPTURE_MODE_CAPTURE);
				retval = CAPTURE_MODE_CAPTURE;
				break;
			default:
				break;
		}

		// Send a reply back to the ARM9
		pxiReply(PxiChannel_User0, retval);
	}

	return 0;
}
