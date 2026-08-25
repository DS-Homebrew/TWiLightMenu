#include "camera.h"
#include "calico/nds/pxi.h"

#include <nds.h>

// https://problemkaputt.de/gbatek-dsi-cameras.htm

Camera activeCamera    = CAM_NONE;
CaptureMode activeMode = CAPTURE_MODE_PREVIEW;

bool cameraInit() {
	REG_SCFG_CLK |= BIT(2); // CamInterfaceClock = ON
	REG_CAM_MCNT = 0;       // Camera Module Control
	swiDelay(0x1E);
	REG_SCFG_CLK |= BIT(8); // CamExternal Clock = ON
	swiDelay(0x1E);
	REG_CAM_MCNT = BIT(1) | BIT(5); // Camera Module Control
	swiDelay(0x2008);
	REG_SCFG_CLK &= ~BIT(8); // CamExternal Clock = OFF
	REG_CAM_CNT &= ~BIT(15); // allow changing params
	REG_CAM_CNT |= BIT(5);   // flush data fifo
	REG_CAM_CNT = (REG_CAM_CNT & ~(0x0300)) | 0x0200;
	REG_CAM_CNT |= BIT(10);
	REG_CAM_CNT |= BIT(11); // irq enable
	REG_SCFG_CLK |= BIT(8); // CamExternal Clock = ON
	swiDelay(0x14);

	// issue "aptina_code_list_init" via I2C bus on ARM7 side
	u32 val = pxiSendAndReceive(PXI_CAMERA, CAM_INIT);

	REG_SCFG_CLK &= ~BIT(8); // CamExternal Clock = OFF
	REG_SCFG_CLK |= BIT(8);  // CamExternal Clock = ON
	swiDelay(0x14);

	return val == 0x2280;
}

bool cameraActivate(Camera cam) {
	if(activeCamera != CAM_NONE)
		cameraDeactivate(activeCamera);

	u32 command = (cam == CAM_INNER) ? CAM0_ACTIVATE : CAM1_ACTIVATE;
	if(pxiSendAndReceive(PXI_CAMERA, command) == command) {
		activeCamera = cam;
		return true;
	} else {
		return false;
	}
}

bool cameraDeactivate(Camera cam) {
	u32 command = (cam == CAM_INNER) ? CAM0_DEACTIVATE : CAM1_DEACTIVATE;
	if(pxiSendAndReceive(PXI_CAMERA, command) == command) {
		activeCamera = CAM_NONE;
		return true;
	} else {
		return false;
	}
}

void cameraTransferStart(u16 *dst, CaptureMode mode) {
	bool preview = mode == CAPTURE_MODE_PREVIEW;

	if(mode != activeMode) {
		pxiSendAndReceive(PXI_CAMERA, preview ? CAM_SET_MODE_PREVIEW : CAM_SET_MODE_CAPTURE);
		activeMode = mode;
	}

	if(REG_CAM_CNT & BIT(15))
		cameraTransferStop();

	if(preview) // enable YUV-to-RGB555 and set "scanline count - 1" to 3
		REG_CAM_CNT |= 0x2003;
	else // raw YUV and "scaline count -1" to 0
		REG_CAM_CNT &= ~0x2003;
	REG_CAM_CNT |= BIT(5);                                 // flush data fifo
	REG_CAM_CNT |= BIT(15);                                // start transfer
	REG_NDMA1SAD  = (u32)&REG_CAM_DAT;                     // source CAM_DTA
	REG_NDMA1DAD  = (u32)dst;                              // dest RAM/VRAM
	REG_NDMA1TCNT = (preview ? 256 * 192 : 640 * 480) / 2; // total length in words
	REG_NDMA1WCNT = preview ? 512 : 320;                   // block length in words
	REG_NDMA1BCNT = 2;                                     // timing interval or so
	REG_NDMA1CNT  = 0x8B044000;                            // start camera DMA
}

void cameraTransferStop() { REG_CAM_CNT &= ~BIT(15); }

bool cameraTransferActive() { return REG_NDMA1CNT & BIT(31); }
