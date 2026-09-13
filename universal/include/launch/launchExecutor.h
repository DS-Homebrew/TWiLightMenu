/*
    launch/launchExecutor.h

    Carries out a LaunchPlan (see launch/launchPlanner.h) on the console. The calling
    menu keeps its own fades, sound and error screens around these steps:

        LaunchEnv env = captureLaunchEnv(ms().secondaryDevice);
        LaunchPlan plan = planLaunch(launcher, env, request);
        applyLaunchSideEffects(plan, &gbaNativeUi);
        applyPlanToSettings(plan, ms().secondaryDevice);
        ms().saveSettings();
        prepareLaunch(plan, true);
        int err = runLaunch(plan, true);	// only returns on failure
*/

#ifndef _LAUNCHEXECUTOR_H_
#define _LAUNCHEXECUTOR_H_

#include "launch/launchPlanner.h"

// Reads console state and settings. secondaryDevice is the ROM's device.
LaunchEnv captureLaunchEnv(bool secondaryDevice);

// How a binary shows progress while a GBA ROM is copied to a Slot-2 cart. Any may be NULL.
struct GbaNativeUi {
	void (*start)(void);		// before anything is written
	void (*progress)(int barLength);	// progress through erasing, then copying: 0 to 192
	void (*copying)(void);		// NOR flash erased, the ROM copy starts
	void (*finish)(void);		// ROM and save copied
};

// Creates DATA_DIRS, writes nds-bootstrap.ini and the DSTWO's twlm.ini, and copies a
// GBA ROM and its save to the Slot-2 cart
void applyLaunchSideEffects(const LaunchPlan &plan, const GbaNativeUi *gbaNativeUi);

// Stores ROM_PATH, LAUNCH_TYPE and HOMEBREW_ARG for the device, without saving
void applyPlanToSettings(const LaunchPlan &plan, bool secondaryDevice);

// Loads nds-bootstrap-hb's bootloader ahead of time when booting it directly.
// Call before fading out; bootstrapDirect is ms().btsrpBootloaderDirect where supported.
void prepareLaunch(const LaunchPlan &plan, bool bootstrapDirect);

// Boots the plan. Returns only on failure, with the error code (1 = nds-bootstrap-hb missing).
int runLaunch(const LaunchPlan &plan, bool bootstrapDirect);

#endif // _LAUNCHEXECUTOR_H_
