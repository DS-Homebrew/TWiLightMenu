#ifndef DSIMENUPP_FLASHCARD_H
#define DSIMENUPP_FLASHCARD_H

extern bool previousUsedDevice;	// true == secondary
extern bool secondaryDevice;
bool sdFound(void);
bool flashcardFound(void);
bool bothSDandFlashcard(void);

#endif //DSIMENUPP_FLASHCARD_H
