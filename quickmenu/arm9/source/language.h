// Language functions.
#ifndef DSIMENUPP_LANGUAGE_H
#define DSIMENUPP_LANGUAGE_H

#include <string>

#define STRING(what,def) extern std::string STR_##what;
#include "language.inl"
#undef STRING

/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 *
 * Check the language variable outside of settings to determine
 * the actual language in use.
 */
void langInit(void);

#endif /* DSIMENUPP_LANGUAGE_H */
