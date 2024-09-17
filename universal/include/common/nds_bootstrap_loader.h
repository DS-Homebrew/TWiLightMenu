/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

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

------------------------------------------------------------------*/

#ifndef NDS_BOOTSTRAP_LOADER_H
#define NDS_BOOTSTRAP_LOADER_H


#ifdef __cplusplus
extern "C" {
#endif

int bootstrapHbRunNds (const void* loader, u32 loaderSize, u32 cluster, u32 ramDiskCluster, u32 ramDiskSize, u32 srParamsCluster, u32 patchOffsetCacheCluster, u32 cfgCluster, u32 cfgSize, int romToRamDisk, bool romIsCompressed, bool initDisc, bool dldiPatchNds, int argc, const char** argv, int language, int dsiMode, bool boostCpu, bool boostVram, int consoleModel, bool soundFreq, u32 srTid1, u32 srTid2, bool ndsPreloaded);

int bootstrapHbRunNdsFile (const char* filename, const char* fatFilename, const char* ramDiskFilename, const char* cfgFilename, u32 ramDiskSize, const char* srParamsFilename, const char* patchOffsetCacheFilename, u32 cfgSize, int romToRamDisk, bool romIsCompressed, int argc, const char** argv, int language, int dsiMode, bool boostCpu, bool boostVram, int consoleModel, bool soundFreq, bool ndsPreloaded);

void bootstrapHbRunPrep (int romToRamDisk);

//bool installBootStub(bool havedsiSD);

#ifdef __cplusplus
}
#endif

#endif // NDS_BOOTSTRAP_LOADER_H
