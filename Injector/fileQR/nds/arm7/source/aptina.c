#include "aptina.h"

#include "aptina_i2c.h"

u8 currentDevice = I2C_CAM0;

// Waits until the bits of the register are clear
void aptWaitClr(u8 device, u16 reg, u16 mask) {
	while(aptReadRegister(device, reg) & mask) {
		swiWaitForVBlank();
	}
}

// Waits until the bits of the register are set
void aptWaitSet(u8 device, u16 reg, u16 mask) {
	while((aptReadRegister(device, reg) & mask) != mask) {
		swiWaitForVBlank();
	}
}

// Clears the bits in the mask at the register
void aptClr(u8 device, u16 reg, u16 mask) {
	u16 temp = aptReadRegister(device, reg);
	aptWriteRegister(device, reg, temp & ~mask);
}

// Sets the bits in the mask at the register
void aptSet(u8 device, u16 reg, u16 mask) {
	u16 temp = aptReadRegister(device, reg);
	aptWriteRegister(device, reg, temp | mask);
}

// Reads an MCU value
u16 aptReadMcu(u8 device, u16 reg) {
	aptWriteRegister(device, 0x098C, reg);
	return aptReadRegister(device, 0x0990);
}

// Writes an MCU value
void aptWriteMcu(u8 device, u16 reg, u16 data) {
	aptWriteRegister(device, 0x098C, reg);
	aptWriteRegister(device, 0x0990, data);
}

// Waits until the bits of the MCU register are clear
void aptWaitMcuClr(u8 device, u16 reg, u16 mask) {
	while(aptReadMcu(device, reg) & mask) {
		swiWaitForVBlank();
	}
}

// Sets the bits in the mask at the MCU register
void aptSetMcu(u8 device, u16 reg, u16 mask) {
	u16 temp = aptReadMcu(device, reg);
	aptWriteMcu(device, reg, temp | mask);
}

// https://problemkaputt.de/gbatek.htm#dsiaptinacamerainitialization
void init(u8 device) {
	aptWriteRegister(device, 0x001A, 0x0003); // RESET_AND_MISC_CONTROL (issue reset)   ;\reset
	aptWriteRegister(device, 0x001A, 0x0000); // RESET_AND_MISC_CONTROL (release reset) ;/
	aptWriteRegister(device, 0x0018, 0x4028); // STANDBY_CONTROL (wakeup)               ;\.
	aptWriteRegister(device, 0x001E, 0x0201); // PAD_SLEW                               ; wakeup
	aptWriteRegister(device, 0x0016, 0x42DF); // CLOCKS_CONTROL                         ;
	aptWaitClr(device, 0x0018, 0x4000);       // STANDBY_CONTROL (wait for WakeupDone)  ;
	aptWaitSet(device, 0x301A, 0x0004);       // UNDOC_CORE_301A (wait for WakeupDone)  ;/
	aptWriteMcu(device, 0x002F0, 0x0000);     // UNDOC! RAM?
	aptWriteMcu(device, 0x002F2, 0x0210);     // UNDOC! RAM?
	aptWriteMcu(device, 0x002F4, 0x001A);     // UNDOC! RAM?
	aptWriteMcu(device, 0x02145, 0x02F4);     // UNDOC! SEQ?
	aptWriteMcu(device, 0x0A134, 0x0001);     // UNDOC! SEQ?
	aptSetMcu(device, 0xA115, 0x0002);        // SEQ_CAP_MODE (set bit1=video)
	aptWriteMcu(device, 0x2755, 0x0002);      // MODE_OUTPUT_FORMAT_A (bit5=0=YUV)      ;\select
	aptWriteMcu(device, 0x2757, 0x0002);      // MODE_OUTPUT_FORMAT_B                   ;/YUV mode
	aptWriteRegister(device, 0x0014, 0x2145); // PLL_CONTROL                            ;\.
	aptWriteRegister(device, 0x0010, 0x0111); // PLL_DIVIDERS                           ; match
	aptWriteRegister(device, 0x0012, 0x0000); // PLL_P_DIVIDERS                         ; PLL
	aptWriteRegister(device, 0x0014, 0x244B); // PLL_CONTROL                            ; to DSi
	aptWriteRegister(device, 0x0014, 0x304B); // PLL_CONTROL                            ; timings
	aptWaitSet(device, 0x0014, 0x8000);       // PLL_CONTROL (wait for PLL Lock okay)   ;
	aptClr(device, 0x0014, 0x0001);           // PLL_CONTROL (disable PLL Bypass)       ;/
	aptWriteMcu(device, 0x2703, 0x0100);      // MODE_OUTPUT_WIDTH_A              ;\Size A
	aptWriteMcu(device, 0x2705, 0x00C0);      // MODE_OUTPUT_HEIGHT_A             ;/ 256x192
	aptWriteMcu(device, 0x2707, 0x0280);      // MODE_OUTPUT_WIDTH_B              ;\Size B
	aptWriteMcu(device, 0x2709, 0x01E0);      // MODE_OUTPUT_HEIGHT_B             ;/ 640x480
	aptWriteMcu(device, 0x2715, 0x0001);      // MODE_SENSOR_ROW_SPEED_A          ;\.
	aptWriteMcu(device, 0x2719, 0x001A);      // MODE_SENSOR_FINE_CORRECTION_A    ;
	aptWriteMcu(device, 0x271B, 0x006B);      // MODE_SENSOR_FINE_IT_MIN_A        ; Sensor A
	aptWriteMcu(device, 0x271D, 0x006B);      // MODE_SENSOR_FINE_IT_MAX_MARGIN_A ;
	aptWriteMcu(device, 0x271F, 0x02C0);      // MODE_SENSOR_FRAME_LENGTH_A       ;
	aptWriteMcu(device, 0x2721, 0x034B);      // MODE_SENSOR_LINE_LENGTH_PCK_A    ;/
	aptWriteMcu(device, 0xA20B, 0x0000);      // AE_MIN_INDEX                     ;\AE min/max
	aptWriteMcu(device, 0xA20C, 0x0006);      // AE_MAX_INDEX                     ;/
	aptWriteMcu(device, 0x272B, 0x0001);      // MODE_SENSOR_ROW_SPEED_B          ;\.
	aptWriteMcu(device, 0x272F, 0x001A);      // MODE_SENSOR_FINE_CORRECTION_B    ;
	aptWriteMcu(device, 0x2731, 0x006B);      // MODE_SENSOR_FINE_IT_MIN_B        ; Sensor B
	aptWriteMcu(device, 0x2733, 0x006B);      // MODE_SENSOR_FINE_IT_MAX_MARGIN_B ;
	aptWriteMcu(device, 0x2735, 0x02C0);      // MODE_SENSOR_FRAME_LENGTH_B       ;
	aptWriteMcu(device, 0x2737, 0x034B);      // MODE_SENSOR_LINE_LENGTH_PCK_B    ;/
	aptSet(device, 0x3210, 0x0008);           // COLOR_PIPELINE_CONTROL (PGA pixel shading..)
	aptWriteMcu(device, 0xA208, 0x0000);      // UNDOC! RESERVED_AE_08
	aptWriteMcu(device, 0xA24C, 0x0020);      // AE_TARGETBUFFERSPEED
	aptWriteMcu(device, 0xA24F, 0x0070);      // AE_BASETARGET
	if(device == I2C_CAM0) {                  //                                  ;\.
		aptWriteMcu(device, 0x2717, 0x0024);  // MODE_SENSOR_READ_MODE_A          ; Read Mode
		aptWriteMcu(device, 0x272D, 0x0024);  // MODE_SENSOR_READ_MODE_B          ; with x-flip
	} else {                                  //                                  ; on internal
		aptWriteMcu(device, 0x2717, 0x0025);  // MODE_SENSOR_READ_MODE_A          ; camera
		aptWriteMcu(device, 0x272D, 0x0025);  // MODE_SENSOR_READ_MODE_B          ;/
	}
	if(device == I2C_CAM0) {                 //                                  ;\.
		aptWriteMcu(device, 0xA202, 0x0022); // AE_WINDOW_POS                    ;
		aptWriteMcu(device, 0xA203, 0x00BB); // AE_WINDOW_SIZE                   ;
	} else {                                 //                                  ;
		aptWriteMcu(device, 0xA202, 0x0000); // AE_WINDOW_POS                    ;
		aptWriteMcu(device, 0xA203, 0x00FF); // AE_WINDOW_SIZE                   ;/
	}
	aptSet(device, 0x0016, 0x0020);               // CLOCKS_CONTROL (set bit5=1, reserved)
	aptWriteMcu(device, 0xA115, 0x0072);          // SEQ_CAP_MODE (was already manipulated above)
	aptWriteMcu(device, 0xA11F, 0x0001);          // SEQ_PREVIEW_1_AWB                ;\.
	if(device == I2C_CAM0) {                      //                                  ;
		aptWriteRegister(device, 0x326C, 0x0900); // ;APERTURE_PARAMETERS             ;
		aptWriteMcu(device, 0xAB22, 0x0001);      // HG_LL_APCORR1                    ;
	} else {                                      //                                  ;
		aptWriteRegister(device, 0x326C, 0x1000); // APERTURE_PARAMETERS              ;
		aptWriteMcu(device, 0xAB22, 0x0002);      // HG_LL_APCORR1                    ;/.
	}
	aptWriteMcu(device, 0xA103, 0x0006);   // SEQ_CMD (06h=RefreshMode)
	aptWaitMcuClr(device, 0xA103, 0x000F); // SEQ_CMD (wait above to become ZERO)
	aptWriteMcu(device, 0xA103, 0x0005);   // SEQ_CMD (05h=Refresh)
	aptWaitMcuClr(device, 0xA103, 0x000F); // SEQ_CMD (wait above to become ZERO)
}

void activate(u8 device) {
	aptClr(device, 0x0018, 0x0001);     // STANDBY_CONTROL (bit0=0=wakeup)       ;\.
	aptWaitClr(device, 0x0018, 0x4000); // STANDBY_CONTROL (wait for WakeupDone) ; Wakeup
	aptWaitSet(device, 0x301A, 0x0004); // UNDOC_CORE_301A (wait for WakeupDone) ;/
	aptWriteRegister(
		device, 0x3012, 0x0010);    // COARSE_INTEGRATION_TIME (Y Time)      ; set to large number for more brightness
	aptSet(device, 0x001A, 0x0200); // RESET_AND_MISC_CONTROL (Parallel On)  ;-Data on
	if(device == I2C_CAM1)
		i2cWriteRegister(0x4A, 0x31, 0x01);

	currentDevice = device;
}

void deactivate(u8 device) {
	aptClr(device, 0x001A, 0x0200);     // RESET_AND_MISC_CONTROL (Parallel Off)  ;-Data off
	aptSet(device, 0x0018, 0x0001);     // STANDBY_CONTROL (set bit0=1=Standby)   ;\.
	aptWaitSet(device, 0x0018, 0x4000); // STANDBY_CONTROL (wait for StandbyDone) ; Standby
	aptWaitClr(device, 0x301A, 0x0004); // UNDOC_CORE_301A (wait for StandbyDone) ;/
	if(device == I2C_CAM1)
		i2cWriteRegister(0x4A, 0x31, 0x00);

	currentDevice = I2C_CAM0;
}

void setMode(CaptureMode mode) {
	aptWriteMcu(currentDevice, 0xA103, mode);
	aptWaitMcuClr(currentDevice, 0xA103, 0xFFFF);
}
