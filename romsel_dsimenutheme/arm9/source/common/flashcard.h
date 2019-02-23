#ifndef DSIMENUPP_FLASHCARD_H
#define DSIMENUPP_FLASHCARD_H

extern bool previousUsedDevice;	// true == secondary
extern bool secondaryDevice;
extern int flashcard;
bool sdFound(void);
bool flashcardFound(void);
bool bothSDandFlashcard(void);
void flashcardInit(void);

#endif //DSIMENUPP_FLASHCARD_H
