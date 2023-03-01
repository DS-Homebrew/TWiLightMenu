/*-----------------------------------------------------------------
 fat.c
 
 NDS MP
 GBAMP NDS Firmware Hack Version 2.12
 An NDS aware firmware patch for the GBA Movie Player.
 By Michael Chisholm (Chishm)
 
 Filesystem code based on GBAMP_CF.c by Chishm (me).
 
License:
 Copyright (C) 2005  Michael "Chishm" Chisholm

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 If you use this code, please give due credit and email me about your
 project at chishm@hotmail.com
------------------------------------------------------------------*/

#include "fat.h"
#include "card.h"


//---------------------------------------------------------------
// FAT constants

#define FILE_LAST 0x00
#define FILE_FREE 0xE5

#define ATTRIB_ARCH	0x20
#define ATTRIB_DIR	0x10
#define ATTRIB_LFN	0x0F
#define ATTRIB_VOL	0x08
#define ATTRIB_HID	0x02
#define ATTRIB_SYS	0x04
#define ATTRIB_RO	0x01

#define FAT16_ROOT_DIR_CLUSTER 0x00

// File Constants
#ifndef EOF
#define EOF -1
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif


//-----------------------------------------------------------------
// FAT constants
#define CLUSTER_EOF_16	0xFFFF

#define ATTRIB_ARCH	0x20
#define ATTRIB_DIR	0x10
#define ATTRIB_LFN	0x0F
#define ATTRIB_VOL	0x08
#define ATTRIB_HID	0x02
#define ATTRIB_SYS	0x04
#define ATTRIB_RO	0x01

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Data Structures

#define __PACKED __attribute__ ((__packed__))

// Boot Sector - must be packed
typedef struct
{
	u8	jmpBoot[3];
	u8	OEMName[8];
	// BIOS Parameter Block
	u16	bytesPerSector;
	u8	sectorsPerCluster;
	u16	reservedSectors;
	u8	numFATs;
	u16	rootEntries;
	u16	numSectorsSmall;
	u8	mediaDesc;
	u16	sectorsPerFAT;
	u16	sectorsPerTrk;
	u16	numHeads;
	u32	numHiddenSectors;
	u32	numSectors;
	union	// Different types of extended BIOS Parameter Block for FAT16 and FAT32
	{
		struct  
		{
			// Ext BIOS Parameter Block for FAT16
			u8	driveNumber;
			u8	reserved1;
			u8	extBootSig;
			u32	volumeID;
			u8	volumeLabel[11];
			u8	fileSysType[8];
			// Bootcode
			u8	bootCode[448];
		}	fat16;
		struct  
		{
			// FAT32 extended block
			u32	sectorsPerFAT32;
			u16	extFlags;
			u16	fsVer;
			u32	rootClus;
			u16	fsInfo;
			u16	bkBootSec;
			u8	reserved[12];
			// Ext BIOS Parameter Block for FAT16
			u8	driveNumber;
			u8	reserved1;
			u8	extBootSig;
			u32	volumeID;
			u8	volumeLabel[11];
			u8	fileSysType[8];
			// Bootcode
			u8	bootCode[420];
		}	fat32;
	}	extBlock;

	__PACKED	u16	bootSig;

}	__PACKED BOOT_SEC;

// Directory entry - must be packed
typedef struct
{
	u8	name[8];
	u8	ext[3];
	u8	attrib;
	u8	reserved;
	u8	cTime_ms;
	u16	cTime;
	u16	cDate;
	u16	aDate;
	u16	startClusterHigh;
	u16	mTime;
	u16	mDate;
	u16	startCluster;
	u32	fileSize;
}	__PACKED DIR_ENT;

// File information - no need to pack
typedef struct
{
	u32 firstCluster;
	u32 length;
	u32 curPos;
	u32 curClus;			// Current cluster to read from
	int curSect;			// Current sector within cluster
	int curByte;			// Current byte within sector
	char readBuffer[512];	// Buffer used for unaligned reads
	u32 appClus;			// Cluster to append to
	int appSect;			// Sector within cluster for appending
	int appByte;			// Byte within sector for appending
	bool read;	// Can read from file
	bool write;	// Can write to file
	bool append;// Can append to file
	bool inUse;	// This file is open
	u32 dirEntSector;	// The sector where the directory entry is stored
	int dirEntOffset;	// The offset within the directory sector
}	FAT_FILE;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Global Variables

// _VARS_IN_RAM variables are stored in the largest section of WRAM 
// available: IWRAM on NDS ARM7, EWRAM on NDS ARM9 and GBA

// Locations on card
int discRootDir;
int discRootDirClus;
int discFAT;
int discSecPerFAT;
int discNumSec;
int discData;
int discBytePerSec;
int discSecPerClus;
int discBytePerClus;

enum {FS_UNKNOWN, FS_FAT12, FS_FAT16, FS_FAT32} discFileSystem;

// Global sector buffer to save on stack space
unsigned char globalBuffer[sizeof(BOOT_SEC)];


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FAT routines

u32 FAT_ClustToSect (u32 cluster) {
	return (((cluster-2) * discSecPerClus) + discData);
}

/*-----------------------------------------------------------------
FAT_NextCluster
Internal function - gets the cluster linked from input cluster
-----------------------------------------------------------------*/
u32 FAT_NextCluster(u32 cluster)
{
	u32 nextCluster = CLUSTER_FREE;
	u32 sector;
	int offset;
	
	
	switch (discFileSystem) 
	{
		case FS_UNKNOWN:
			nextCluster = CLUSTER_FREE;
			break;
			
		case FS_FAT12:
			sector = discFAT + (((cluster * 3) / 2) / BYTES_PER_SECTOR);
			offset = ((cluster * 3) / 2) % BYTES_PER_SECTOR;
			CARD_ReadSector(sector, globalBuffer);
			nextCluster = ((u8*) globalBuffer)[offset];
			offset++;
			
			if (offset >= BYTES_PER_SECTOR) {
				offset = 0;
				sector++;
			}
			
			CARD_ReadSector(sector, globalBuffer);
			nextCluster |= (((u8*) globalBuffer)[offset]) << 8;
			
			if (cluster & 0x01) {
				nextCluster = nextCluster >> 4;
			} else 	{
				nextCluster &= 0x0FFF;
			}
			
			break;
			
		case FS_FAT16:
			sector = discFAT + ((cluster << 1) / BYTES_PER_SECTOR);
			offset = cluster % (BYTES_PER_SECTOR >> 1);
			
			CARD_ReadSector(sector, globalBuffer);
			// read the nextCluster value
			nextCluster = ((u16*)globalBuffer)[offset];
			
			if (nextCluster >= 0xFFF7) {
				nextCluster = CLUSTER_EOF;
			}
			break;
			
		case FS_FAT32:
			sector = discFAT + ((cluster << 2) / BYTES_PER_SECTOR);
			offset = cluster % (BYTES_PER_SECTOR >> 2);
			
			CARD_ReadSector(sector, globalBuffer);
			// read the nextCluster value
			nextCluster = (((u32*)globalBuffer)[offset]) & 0x0FFFFFFF;
			
			if (nextCluster >= 0x0FFFFFF7) {
				nextCluster = CLUSTER_EOF;
			}
			break;
			
		default:
			nextCluster = CLUSTER_FREE;
			break;
	}
	
	return nextCluster;
}

/*-----------------------------------------------------------------
ucase
Returns the uppercase version of the given char
char IN: a character
char return OUT: uppercase version of character
-----------------------------------------------------------------*/
char ucase (char character)
{
	if ((character > 0x60) && (character < 0x7B))
		character = character - 0x20;
	return (character);
}

/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (bool initCard)
{
	int i;
	int bootSector;
	BOOT_SEC* bootSec;
	
	if (initCard && !CARD_StartUp()) {
		return (false);
	}
	
	// Read first sector of card
	if (!CARD_ReadSector (0, globalBuffer)) 
	{
		return false;
	}

	if (((globalBuffer[0x36] == 'F') && (globalBuffer[0x37] == 'A') && (globalBuffer[0x38] == 'T')) // Check if there is a FAT string, which indicates this is a boot sector
	 || ((globalBuffer[0x52] == 'F') && (globalBuffer[0x53] == 'A') && (globalBuffer[0x54] == 'T'))) { // Check for FAT32
		bootSector = 0;
	} else	// This is an MBR
	{
		// Find first valid partition from MBR
		// First check for an active partition
		for (i=0x1BE; (i < 0x1FE) && (globalBuffer[i] != 0x80); i+= 0x10);
		// If it didn't find an active partition, search for any valid partition
		if (i == 0x1FE) 
			for (i=0x1BE; (i < 0x1FE) && (globalBuffer[i+0x04] == 0x00); i+= 0x10);
		
		// Go to first valid partition
		if (i != 0x1FE)	// Make sure it found a partition
		{
			bootSector = globalBuffer[0x8 + i] + (globalBuffer[0x9 + i] << 8) + (globalBuffer[0xA + i] << 16) + ((globalBuffer[0xB + i] << 24) & 0x0F);
		} else {
			bootSector = 0;	// No partition found, assume this is a MBR free disk
		}
	}

	// Read in boot sector
	bootSec = (BOOT_SEC*) globalBuffer;
	CARD_ReadSector (bootSector,  bootSec);
	
	// Store required information about the file system
	if (bootSec->sectorsPerFAT != 0) {
		discSecPerFAT = bootSec->sectorsPerFAT;
	} else {
		discSecPerFAT = bootSec->extBlock.fat32.sectorsPerFAT32;
	}
	
	if (bootSec->numSectorsSmall != 0) {
		discNumSec = bootSec->numSectorsSmall;
	} else {
		discNumSec = bootSec->numSectors;
	}

	discBytePerSec = BYTES_PER_SECTOR;	// Sector size is redefined to be 512 bytes
	discSecPerClus = bootSec->sectorsPerCluster * bootSec->bytesPerSector / BYTES_PER_SECTOR;
	discBytePerClus = discBytePerSec * discSecPerClus;
	discFAT = bootSector + bootSec->reservedSectors;

	discRootDir = discFAT + (bootSec->numFATs * discSecPerFAT);
	discData = discRootDir + ((bootSec->rootEntries * sizeof(DIR_ENT)) / BYTES_PER_SECTOR);

	if ((discNumSec - discData) / bootSec->sectorsPerCluster < 4085) {
		discFileSystem = FS_FAT12;
	} else if ((discNumSec - discData) / bootSec->sectorsPerCluster < 65525) {
		discFileSystem = FS_FAT16;
	} else {
		discFileSystem = FS_FAT32;
	}

	if (discFileSystem != FS_FAT32) {
		discRootDirClus = FAT16_ROOT_DIR_CLUSTER;
	} else	// Set up for the FAT32 way
	{
		discRootDirClus = bootSec->extBlock.fat32.rootClus;
		// Check if FAT mirroring is enabled
		if (!(bootSec->extBlock.fat32.extFlags & 0x80)) {
			// Use the active FAT
			discFAT = discFAT + (discSecPerFAT * (bootSec->extBlock.fat32.extFlags & 0x0F));
		}
	}

	return (true);
}


/*-----------------------------------------------------------------
getBootFileCluster
-----------------------------------------------------------------*/
u32 getBootFileCluster (const char* bootName)
{
	DIR_ENT dir;
	int firstSector = 0;
	bool notFound = false;
	bool found = false;
//	int maxSectors;
	u32 wrkDirCluster = discRootDirClus;
	u32 wrkDirSector = 0;
	int wrkDirOffset = 0;
	int nameOffset;
	
	dir.startCluster = CLUSTER_FREE; // default to no file found
	dir.startClusterHigh = CLUSTER_FREE;
	

	// Check if fat has been initialised
	if (discBytePerSec == 0) {
		return (CLUSTER_FREE);
	}
	
	char *ptr = (char*)bootName;
	while (*ptr != '.') ptr++;
	int namelen = ptr - bootName;

//	maxSectors = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? (discData - discRootDir) : discSecPerClus);
	// Scan Dir for correct entry
	firstSector = discRootDir;
	CARD_ReadSector (firstSector + wrkDirSector, globalBuffer);
	found = false;
	notFound = false;
	wrkDirOffset = -1;	// Start at entry zero, Compensating for increment
	while (!found && !notFound) {
		wrkDirOffset++;
		if (wrkDirOffset == BYTES_PER_SECTOR / sizeof (DIR_ENT)) {
			wrkDirOffset = 0;
			wrkDirSector++;
			if ((wrkDirSector == discSecPerClus) && (wrkDirCluster != FAT16_ROOT_DIR_CLUSTER)) {
				wrkDirSector = 0;
				wrkDirCluster = FAT_NextCluster(wrkDirCluster);
				if (wrkDirCluster == CLUSTER_EOF) {
					notFound = true;
				}
				firstSector = FAT_ClustToSect(wrkDirCluster);		
			} else if ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER) && (wrkDirSector == (discData - discRootDir))) {
				notFound = true;	// Got to end of root dir
			}
			CARD_ReadSector (firstSector + wrkDirSector, globalBuffer);
		}
		dir = ((DIR_ENT*) globalBuffer)[wrkDirOffset];
		found = true;
		if ((dir.attrib & ATTRIB_DIR) || (dir.attrib & ATTRIB_VOL)) {
			found = false;
		}
		if (namelen<8 && dir.name[namelen]!=0x20) found = false;
		for (nameOffset = 0; nameOffset < namelen && found; nameOffset++) {
			if (ucase(dir.name[nameOffset]) != bootName[nameOffset])
				found = false;
		}
		for (nameOffset = 0; nameOffset < 3 && found; nameOffset++) {
			if (ucase(dir.ext[nameOffset]) != bootName[nameOffset+namelen+1])
				found = false;
		}
		if (dir.name[0] == FILE_LAST) {
			notFound = true;
		}
	} 
	
	// If no file is found, return CLUSTER_FREE
	if (notFound) {
		return CLUSTER_FREE;
	}

	return (dir.startCluster | (dir.startClusterHigh << 16));
}

/*-----------------------------------------------------------------
fileRead(buffer, cluster, startOffset, length)
-----------------------------------------------------------------*/
u32 fileRead (char* buffer, u32 cluster, u32 startOffset, u32 length)
{
	int curByte;
	int curSect;
	
	int dataPos = 0;
	int chunks;
	int beginBytes;

	if (cluster == CLUSTER_FREE || cluster == CLUSTER_EOF) 
	{
		return 0;
	}
	
	// Follow cluster list until desired one is found
	for (chunks = startOffset / discBytePerClus; chunks > 0; chunks--) {
		cluster = FAT_NextCluster (cluster);
	}
	
	// Calculate the sector and byte of the current position,
	// and store them
	curSect = (startOffset % discBytePerClus) / BYTES_PER_SECTOR;
	curByte = startOffset % BYTES_PER_SECTOR;

	// Load sector buffer for new position in file
	CARD_ReadSector(curSect + FAT_ClustToSect(cluster), globalBuffer);
	curSect++;

	// Number of bytes needed to read to align with a sector
	beginBytes = (BYTES_PER_SECTOR < length + curByte ? (BYTES_PER_SECTOR - curByte) : length);

	// Read first part from buffer, to align with sector boundary
	for (dataPos = 0 ; dataPos < beginBytes; dataPos++) {
		buffer[dataPos] = globalBuffer[curByte++];
	}

	// Read in all the 512 byte chunks of the file directly, saving time
	for (chunks = ((int)length - beginBytes) / BYTES_PER_SECTOR; chunks > 0;) {
		int sectorsToRead;

		// Move to the next cluster if necessary
		if (curSect >= discSecPerClus) {
			curSect = 0;
			cluster = FAT_NextCluster (cluster);
		}

		// Calculate how many sectors to read (read a maximum of discSecPerClus at a time)
		sectorsToRead = discSecPerClus - curSect;
		if (chunks < sectorsToRead)
			sectorsToRead = chunks;

		// Read the sectors
		CARD_ReadSectors(curSect + FAT_ClustToSect(cluster), sectorsToRead, buffer + dataPos);
		chunks  -= sectorsToRead;
		curSect += sectorsToRead;
		dataPos += BYTES_PER_SECTOR * sectorsToRead;
	}

	// Take care of any bytes left over before end of read
	if (dataPos < length) {

		// Update the read buffer
		curByte = 0;
		if (curSect >= discSecPerClus) {
			curSect = 0;
			cluster = FAT_NextCluster (cluster);
		}
		CARD_ReadSector( curSect + FAT_ClustToSect( cluster), globalBuffer);
		
		// Read in last partial chunk
		for (; dataPos < length; dataPos++) {
			buffer[dataPos] = globalBuffer[curByte];
			curByte++;
		}
	}
	
	return dataPos;
}
