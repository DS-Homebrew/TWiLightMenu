#ifndef DSIMENUPP_SYSTEM_H
#define DSIMENUPP_SYSTEM_H

/**
	Check for DSi mode a different way (separate from libnds function)
	(Partial is DSi mode with DS BIOS set)
**/
static inline bool isDSiMode_partial(void) {
	return (REG_SCFG_EXT == 0x8307F100);
};

#endif /* DSIMENUPP_SYSTEM_H */
