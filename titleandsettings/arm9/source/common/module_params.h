#include <nds.h>
#include <stdio.h>
#include "ndsheader.h"
#pragma once
#ifndef __MODULE_PARAM__
#define __MODULE_PARAM__
typedef struct 
{
	u32 auto_load_list_offset;
	u32 auto_load_list_end;
	u32 auto_load_start;
	u32 static_bss_start;
	u32 static_bss_end;
	u32 compressed_static_end;
	u32 sdk_version;
	u32 nitro_code_be;
	u32 nitro_code_le;
} module_params_t;

module_params_t* getModuleParams(const sNDSHeaderExt* ndsHeader, FILE* ndsFile);
#endif