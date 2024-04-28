#include <nds.h>
#include <stdio.h>
#include "ndsheaderbanner.h"

#pragma once
#ifndef __MODULE_PARAM__
#define __MODULE_PARAM__
/**
 * Gets a pointer to module params of a file, valid until
 * the next call of getModuleParams.
 * 
 * Try to ensure that the NDS ROM actually has moduleparams,
 * (i.e. not homebrew), else this method will be slow.
 */
module_params_t* getModuleParams(const sNDSHeaderExt* ndsHeader, FILE* ndsFile);
#endif