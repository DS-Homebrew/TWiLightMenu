#include <nds.h>

int PowerOnSlot() {
    REG_SCFG_MC = 0x04;    // set state=1
    while(REG_SCFG_MC&1);
    
    REG_SCFG_MC = 0x08;    // set state=2      
    while(REG_SCFG_MC&1);
    
    REG_ROMCTRL = 0x20000000; // set ROMCTRL=20000000h
    return 0;
}

int PowerOffSlot() {
    if(REG_SCFG_MC&1) return 1; 
    
    REG_SCFG_MC = 0x0C; // set state=3 
    while(REG_SCFG_MC&1);
    return 0;
}

int TWL_ResetSlot1() {
	PowerOffSlot();
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }
	PowerOnSlot();
	return 0;
}

