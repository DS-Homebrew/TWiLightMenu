#include <nds.h>
#include <fat.h>
#include <sys/stat.h>
#include <stdio.h>

static bool sdAccessed = false;
static bool sdRead = false;

static bool flashcardAccessed = false;
static bool flashcardRead = false;

bool sdFound(void) {
	if (!sdAccessed) {
		if (access("sd:/", F_OK) == 0) {
			sdRead = true;
		} else {
			sdRead = false;
		}
		sdAccessed = true;
	}
	return sdRead;
}

bool flashcardFound(void) {
	if (!flashcardAccessed) {
		if (access("fat:/", F_OK) == 0) {
			flashcardRead = true;
		} else {
			flashcardRead = false;
		}
		flashcardAccessed = true;
	}
	return flashcardRead;
}

bool bothSDandFlashcard(void) {
	if (sdFound() && flashcardFound()) {
		return true;
	} else {
		return false;
	}
}