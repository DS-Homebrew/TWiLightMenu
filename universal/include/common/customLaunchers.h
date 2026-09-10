/*
    common/customLaunchers.h

    User-definable file-type launchers.

    A user may drop "config.<ext>.ini" files into "/_nds/TWiLightMenu/extras" to
    teach TWiLight Menu++ about a file extension it does not handle natively:

        [LAUNCHER]
        LAUNCHER_PATH=/_nds/TWiLightMenu/apps/MyViewer.nds
        BANNER_PATH=/_nds/TWiLightMenu/icons/MyIcon.bin
        ARG=%PATH%
        EXTRA_ARGS=
        BOOST_CPU=1
        BOOST_VRAM=-1
        DS_MODE=0

    Configs may only add *new* extensions; one naming an extension TWiLight Menu++
    already handles is ignored, as is one whose LAUNCHER_PATH is not on the card.
*/

#ifndef _CUSTOMLAUNCHERS_H_
#define _CUSTOMLAUNCHERS_H_

#include <string>
#include <string_view>
#include <vector>

struct CustomLauncher {
	std::string extension;    // ".ext", lowercased, with the leading dot
	std::string launcherPath; // resolved, "sd:/..." or "fat:/..."
	std::string bannerPath;   // resolved, or empty if none was given
	bool bannerIsPng;         // false = raw NDS banner (.bin), true = .png
	std::string arg;          // argv template, "" means pass no argument
	std::string extraArgs;    // space separated literal args, placed before arg
	int boostCpu;             // 0/1
	int boostVram;            // 0/1, or -1 for "auto" (see findCustomLauncher users)
	int dsMode;               // 0/1, maps to dsModeSwitch
};

// Scans the extras folder once. Safe to call more than once; later calls are no-ops.
void loadCustomLaunchers(void);

// The loaded configs. Their strings stay alive for the rest of the session, so
// std::string_view's into them (e.g. in extensionList) remain valid.
const std::vector<CustomLauncher> &customLaunchers(void);

// Matches filename's extension against the loaded configs, case-insensitively.
const CustomLauncher *findCustomLauncher(std::string_view filename);

// Expands the ARG template. romPath is the file as launched ("sd:/roms/x/a.ext"),
// romPathFat the same in "fat:/" form, filename the bare name with extension.
std::string buildLauncherArg(const CustomLauncher &launcher, const std::string &romPath,
			     const std::string &romPathFat, const std::string &filename);

// Splits EXTRA_ARGS on spaces. Returns an empty vector when there are none.
std::vector<std::string> splitLauncherExtraArgs(const CustomLauncher &launcher);

#endif // _CUSTOMLAUNCHERS_H_
