#include "gift.h"

#include <limits.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <nds.h>

#include "common/systemdetails.h"
#include "common/twlmenusettings.h"

// Defined in ndsheaderbanner.cpp
extern bool isDirectory[40];
extern bool bnrSysSettings[41];

bool giftWrapped[40] = {false};
int giftAnimFrame[40] = {0};

// Full paths of every app that has been unwrapped
static std::set<std::string> seenApps;
static bool seenListLoaded = false;

static const char *seenListPath(void) {
	return sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/giftseen.txt" : "fat:/_nds/TWiLightMenu/giftseen.txt";
}

// Apps are remembered by folder + file name, so the same file name in two
// different folders is two different apps.
static std::string entryKey(const std::string &name) {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		return name;
	}
	std::string key(cwd);
	if (key.empty() || key[key.size() - 1] != '/') {
		key += '/';
	}
	return key + name;
}

void giftLoadSeenList() {
	if (seenListLoaded) {
		return;
	}
	seenListLoaded = true;

	FILE *file = fopen(seenListPath(), "r");
	if (!file) {
		return; // No list yet: every app is new
	}
	char line[PATH_MAX + 2];
	while (fgets(line, sizeof(line), file)) {
		size_t len = strlen(line);
		while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
			line[--len] = '\0';
		}
		if (len > 0) {
			seenApps.insert(std::string(line));
		}
	}
	fclose(file);
}

void giftClearSlots() {
	for (int i = 0; i < 40; i++) {
		giftWrapped[i] = false;
		giftAnimFrame[i] = 0;
	}
}

void giftUpdateSlot(int slot, bool isDir, const std::string &name) {
	giftAnimFrame[slot] = 0;
	if (ms().theme != TWLSettings::ETheme3DS || isDir) {
		giftWrapped[slot] = false;
		return;
	}
	giftLoadSeenList();
	giftWrapped[slot] = (seenApps.find(entryKey(name)) == seenApps.end());
}

bool giftActive(int slot) {
	return giftWrapped[slot] && !isDirectory[slot] && !bnrSysSettings[slot];
}

int giftSpriteFrame(int slot) {
	const int t = giftAnimFrame[slot];
	if (t < 24) {
		return 0; // Closed (shaking)
	} else if (t < 30) {
		return 1; // Lid starts to lift
	} else if (t < 36) {
		return 2; // Lid up, light shining out
	}
	return 3; // Burst of light
}

int giftShakeX(int slot) {
	const int t = giftAnimFrame[slot];
	if (t <= 0 || t >= 24) {
		return 0;
	}
	const int amplitude = (t < 12) ? 2 : 3;
	return ((t / 2) % 2) ? amplitude : -amplitude;
}

void giftFinishUnwrap(int slot, const std::string &name) {
	const std::string key = entryKey(name);
	if (seenApps.insert(key).second) {
		FILE *file = fopen(seenListPath(), "a");
		if (file) {
			fprintf(file, "%s\n", key.c_str());
			fclose(file);
		}
	}
	giftWrapped[slot] = false;
	giftAnimFrame[slot] = 0;
}
