#ifndef MODULE_PARAMS_H
#define MODULE_PARAMS_H

#include <nds/ndstypes.h>

// Not precise, since it doesn't really matter...
#define FIRST_SDK5_VERSION 0x05000000
#define LAST_NON_SDK5_VERSION (FIRST_SDK5_VERSION - 1)

typedef struct {
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

inline bool isSdk5(const module_params_t* moduleParams) {
	return (moduleParams == NULL) || (moduleParams->sdk_version >= FIRST_SDK5_VERSION);
}

#endif // MODULE_PARAMS_H
