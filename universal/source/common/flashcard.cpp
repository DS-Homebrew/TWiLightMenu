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
