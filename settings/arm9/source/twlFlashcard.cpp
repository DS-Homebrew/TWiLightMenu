#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <sys/stat.h>
#include <stdio.h>

#include "common/flashcard.h"
#include "common/bootstrappaths.h"
#include "common/inifile.h"
#include "common/lzss.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "read_card.h"

/*TWL_CODE bool UpdateCardInfo(char* gameid, char* gamename) {
	memcpy(gameid, ndsCardHeader.gameCode, 4);
	gameid[4] = 0x00;
	memcpy(gamename, ndsCardHeader.gameTitle, 12);
	gamename[12] = 0x00;
	return true;
}

TWL_CODE void ShowGameInfo(const char gameid[], const char gamename[]) {
	iprintf("Game id: %s\nName:    %s", gameid, gamename);
}*/

typedef signed int addr_t;
typedef unsigned char data_t;

#define FIX_ALL	0x01
#define FIX_GLUE	0x02
#define FIX_GOT	0x04
#define FIX_BSS	0x08

enum DldiOffsets {
	DO_magicString = 0x00,			// "\xED\xA5\x8D\xBF Chishm"
	DO_magicToken = 0x00,			// 0xBF8DA5ED
	DO_magicShortString = 0x04,		// " Chishm"
	DO_version = 0x0C,
	DO_driverSize = 0x0D,
	DO_fixSections = 0x0E,
	DO_allocatedSpace = 0x0F,

	DO_friendlyName = 0x10,

	DO_text_start = 0x40,			// Data start
	DO_data_end = 0x44,				// Data end
	DO_glue_start = 0x48,			// Interworking glue start	-- Needs address fixing
	DO_glue_end = 0x4C,				// Interworking glue end
	DO_got_start = 0x50,			// GOT start					-- Needs address fixing
	DO_got_end = 0x54,				// GOT end
	DO_bss_start = 0x58,			// bss start					-- Needs setting to zero
	DO_bss_end = 0x5C,				// bss end

	// IO_INTERFACE data
	DO_ioType = 0x60,
	DO_features = 0x64,
	DO_startup = 0x68,	
	DO_isInserted = 0x6C,	
	DO_readSectors = 0x70,	
	DO_writeSectors = 0x74,
	DO_clearStatus = 0x78,
	DO_shutdown = 0x7C,
	DO_code = 0x80
};

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

TWL_CODE static void dldiRelocateBinary (data_t *binData, size_t dldiFileSize)
{
	addr_t memOffset;			// Offset of DLDI after the file is loaded into memory
	addr_t relocationOffset;	// Value added to all offsets within the patch to fix it properly
	addr_t ddmemOffset;			// Original offset used in the DLDI file
	addr_t ddmemStart;			// Start of range that offsets can be in the DLDI file
	addr_t ddmemEnd;			// End of range that offsets can be in the DLDI file
	addr_t ddmemSize;			// Size of range that offsets can be in the DLDI file

	addr_t addrIter;

	data_t *pDH = binData;
	data_t *pAH = (data_t*)(io_dldi_data);

	// size_t dldiFileSize = 1 << pDH[DO_driverSize];

	memOffset = readAddr (pAH, DO_text_start);
	if (memOffset == 0) {
			memOffset = readAddr (pAH, DO_startup) - DO_code;
	}
	ddmemOffset = readAddr (pDH, DO_text_start);
	relocationOffset = memOffset - ddmemOffset;

	ddmemStart = readAddr (pDH, DO_text_start);
	ddmemSize = (1 << pDH[DO_driverSize]);
	ddmemEnd = ddmemStart + ddmemSize;

	// Remember how much space is actually reserved
	pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
	// Copy the DLDI patch into the application
	tonccpy (pAH, pDH, dldiFileSize);

	// Fix the section pointers in the header
	writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
	writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
	writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
	writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
	writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
	writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
	writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
	writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
	// Fix the function pointers in the header
	writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
	writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
	writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
	writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
	writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
	writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

	if (pDH[DO_fixSections] & FIX_ALL) { 
		// Search through and fix pointers within the data section of the file
		for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GLUE) { 
		// Search through and fix pointers within the glue section of the file
		for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_GOT) { 
		// Search through and fix pointers within the Global Offset Table section of the file
		for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
			if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
				writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
			}
		}
	}

	if (pDH[DO_fixSections] & FIX_BSS) { 
		// Initialise the BSS to 0
		toncset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
	}
}

TWL_CODE void myDldiLoadFromFile (const char* filename) {
	extern bool extension(const std::string_view filename, const std::vector<std::string_view> extensions);

	u32* dldiAddr = new u32[0x8000/sizeof(u32)];

	FILE* file = fopen(filename, "rb");
	if (extension(filename, {".lz77"})) {
		fread((void*)0x02FF8004, 1, 0x3FFC, file);

		*(u32*)0x02FF8000 = 0x53535A4C;
		LZ77_Decompress((u8*)0x02FF8004, (u8*)dldiAddr);
	} else {
		fread(dldiAddr, 1, 0x8000, file);
	}
	fclose(file);

	// Check that it is a valid DLDI
	if (!dldiIsValid ((DLDI_INTERFACE*)dldiAddr)) {
		delete[] dldiAddr;
		return;
	}

	DLDI_INTERFACE* device = (DLDI_INTERFACE*)dldiAddr;
	size_t dldiSize;

	// Calculate actual size of DLDI
	// Although the file may only go to the dldiEnd, the BSS section can extend past that
	if (device->dldiEnd > device->bssEnd) {
		dldiSize = (char*)device->dldiEnd - (char*)device->dldiStart;
	} else {
		dldiSize = (char*)device->bssEnd - (char*)device->dldiStart;
	}
	dldiSize = (dldiSize + 0x03) & ~0x03; 		// Round up to nearest integer multiple
	
	dldiRelocateBinary ((data_t*)dldiAddr, dldiSize);
	delete[] dldiAddr;
}

TWL_CODE const DISC_INTERFACE *dldiGet(void) {
	if(io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA)
		sysSetCartOwner(BUS_OWNER_ARM9);
	if(io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)
		sysSetCardOwner(BUS_OWNER_ARM9);

	return &io_dldi_data->ioInterface;
}

TWL_CODE void twl_flashcardInit(void) {
	if (REG_SCFG_MC != 0x11 && !sys().arm7SCFGLocked()) {
		// Reset Slot-1 to allow reading title name and ID
		my_cardReset(false);

		CIniFile settingsini( DSIMENUPP_INI );

		if (settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", 0) == false) {
			disableSlot1();
			return;
		}

		// Read title name and ID
		cardInit();

		/*char gamename[13];
		char gameid[5];

		UpdateCardInfo(&gameid[0], &gamename[0]);

		consoleDemoInit();
		iprintf("REG_SCFG_MC: %x\n", REG_SCFG_MC);
		ShowGameInfo(gameid, gamename);

		for (int i = 0; i < 60*5; i++) {
			swiWaitForVBlank();
		}*/

		sysSetCardOwner (BUS_OWNER_ARM7);	// 3DS fix

		// Read a DLDI driver specific to the cart
		/*if (!memcmp(ndsCardHeader.gameCode, "ASMA", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/r4tf.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "TOP TF/SD DS", 12) || !memcmp(ndsCardHeader.gameCode, "A76E", 4) || ((u32)ndsCardHeader.gameCode == 0xB003C24)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/ttio.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "PASS", 4) && !memcmp(ndsCardHeader.gameCode, "ASME", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/CycloEvo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameTitle, "D!S!XTREME", 12) && !memcmp(ndsCardHeader.gameCode, "AYIE", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/dsx.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else*/ if (!memcmp(ndsCardHeader.gameTitle, "QMATETRIAL", 9) || !memcmp(ndsCardHeader.gameTitle, "R4DSULTRA", 9) // R4iDSN/R4 Ultra
				|| !memcmp(ndsCardHeader.gameCode, "ACEK", 4) || !memcmp(ndsCardHeader.gameCode, "YCEP", 4) || !memcmp(ndsCardHeader.gameCode, "AHZH", 4) || !memcmp(ndsCardHeader.gameCode, "CHPJ", 4) || !memcmp(ndsCardHeader.gameCode, "ADLP", 4)) { // Acekard 2(i)
			myDldiLoadFromFile("nitro:/dldi/ak2.dldi");
			fatMountSimple("fat", dldiGet());
		} else if (!memcmp(ndsCardHeader.gameCode, "DSGB", 4)) {
			myDldiLoadFromFile("nitro:/dldi/nrio.lz77");
			fatMountSimple("fat", dldiGet());
		} /*else if (!memcmp(ndsCardHeader.gameCode, "ALXX", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/dstwo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} else if (!memcmp(ndsCardHeader.gameCode, "VCKF", 4)) {
			io_dldi_data = dldiLoadFromFile("nitro:/dldi/CycloIEvo.dldi");
			fatMountSimple("fat", &io_dldi_data->ioInterface);
		} */

		flashcardFoundReset();
		if (!flashcardFound()) {
			disableSlot1();
		}
	}
}

void flashcardInit(void) {
	if (isDSiMode() && !flashcardFound()) {
		twl_flashcardInit();
	}
}
