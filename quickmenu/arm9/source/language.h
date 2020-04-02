// Language functions.
#ifndef DSIMENUPP_LANGUAGE_H
#define DSIMENUPP_LANGUAGE_H

extern int setLanguage;
extern int setGameLanguage;
extern int setTitleLanguage;

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 *
 * Check the language variable outside of settings to determine
 * the actual language in use.
 */
void langInit(void);

#endif /* DSIMENUPP_LANGUAGE_H */
