#ifndef FAT_HEADER_H
#define FAT_HEADER_H

// From NTM
// https://github.com/Epicpkmn11/NTM/blob/c05199c59a843979a0acf14dd9dabb159e3ed103/arm9/src/sav.h#L9-L33
typedef struct
{
	u8 BS_JmpBoot[3];	//0x0000
	u8 BS_OEMName[8];	//0x0003
	u16 BPB_BytesPerSec;	//0x000B
	u8 BPB_SecPerClus;	//0x000D
	u16 BPB_RsvdSecCnt;	//0x000E
	u8 BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_TotSec16;
	u8 BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32 BPB_HiddSec;
	u32 BPB_TotSec32;
	u8 BS_DrvNum;
	u8 BS_Reserved1;
	u8 BS_BootSig;
	u32 BS_VolID;
	u8 BS_VolLab[11];
	u8 BS_FilSysType[8];
	u8 BS_BootCode[448];
	u16 BS_BootSign;
} __attribute__ ((__packed__)) FATHeader;

#endif // FAT_HEADER_H