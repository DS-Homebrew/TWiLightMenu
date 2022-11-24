#include <fat.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static bool sdAccessed = false;
static bool sdRead = false;

static bool flashcardAccessed = false;
static bool flashcardRead = false;

bool sdFound(void) {
	if (!sdAccessed) {
		sdRead = (access("sd:/", F_OK) == 0);
		sdAccessed = true;
	}
	return sdRead;
}

bool flashcardFound(void) {
	if (!flashcardAccessed) {
		flashcardRead = (access("fat:/", F_OK) == 0);
		flashcardAccessed = true;
	}
	return flashcardRead;
}

void flashcardFoundReset(void) {
	flashcardAccessed = false;
}

bool bothSDandFlashcard(void) {
	return (sdFound() && flashcardFound());
}

bool isRunFromSd(void) {
	static bool checked = false;
	static bool result = false;
	if (!checked) {
		if (flashcardFound()) {
			if (access("fat:/_nds/primary", F_OK) == 0 || !sdFound()) {
				result = false;
			} else {
				result = (access("fat:/_nds/TWiLightMenu/main.srldr", F_OK) != 0);
			}
		} else {
			result = (access("sd:/_nds/TWiLightMenu/main.srldr", F_OK) == 0);
		}
		checked = true;
	}
	return result;
}
