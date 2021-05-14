#include <nds.h>
#include <slim.h>
#include <sys/stat.h>
#include <stdio.h>

bool sdFound(void) {
	return (access("sd:/", F_OK) == 0);
}

bool flashcardFound(void) {
	return (access("fat:/", F_OK) == 0);
}

bool bothSDandFlashcard(void) {
	return (sdFound() && flashcardFound());
}