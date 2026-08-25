#ifndef CAMERA_H
#define CAMERA_H

#include "pxi_vars.h"

#include <nds/ndstypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REG_CAM_MCNT *(vu16 *)0x04004200
#define REG_CAM_CNT *(vu16 *)0x04004202
#define REG_CAM_DAT *(vu16 *)0x04004204
#define REG_NDMA1SAD *(vu32 *)0x04004120
#define REG_NDMA1DAD *(vu32 *)0x04004124
#define REG_NDMA1TCNT *(vu32 *)0x04004128
#define REG_NDMA1WCNT *(vu32 *)0x0400412C
#define REG_NDMA1BCNT *(vu32 *)0x04004130
#define REG_NDMA1CNT *(vu32 *)0x04004138

typedef enum { CAM_NONE, CAM_INNER, CAM_OUTER } Camera;

// Initializes the cameras
bool cameraInit(void);

// Activates a camera, the other is automatically deactivated if active
bool cameraActivate(Camera cam);

// Deactivates a camera
bool cameraDeactivate(Camera cam);

// Starts transfer from the camera to the specified RAM/VRAM using NDMA 1
void cameraTransferStart(u16 *dst, CaptureMode mode);

// Stops camera transfer
void cameraTransferStop(void);

// Checks if a camera transfer is active
bool cameraTransferActive(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
