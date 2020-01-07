#include "loaderconfig.h"
#include "dsimenusettings.h"

#pragma once
#ifndef __FLASHCARD_LAUNCH_H___
#define __FLASHCARD_LAUNCH_H___

int loadGameOnFlashcard(const char *ndsPath, std::string filename, bool usePerGameSettings);

#endif