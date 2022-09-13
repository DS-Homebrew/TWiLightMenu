/*
    exptools.cpp
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "exptools.h"

#define _PSRAM 0x08060000 // an offset into PSRAM to write to so stuff doesn't get lost...

void cExpansion::OpenNorWrite(void)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9c40000=0x1500;
  *(vu16*)0x9fc0000=0x1500;
}

void cExpansion::CloseNorWrite(void)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9c40000=0xd200;
  *(vu16*)0x9fc0000=0x1500;
}

void cExpansion::SetRompage(u16 page)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9880000=page;
  *(vu16*)0x9fc0000=0x1500;
}

void cExpansion::SetRampage(u16 page)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9c00000=page;
  *(vu16*)0x9fc0000=0x1500;
  iRamPage=page;
}

u16 cExpansion::Rampage(void)
{
  return iRamPage;
}

void cExpansion::SetSerialMode(void)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9a40000=0xe200;
  *(vu16*)0x9fc0000=0x1500;
}

void cExpansion::SetShake(u16 data)
{
  *(vu16*)0x9fe0000=0xd200;
  *(vu16*)0x8000000=0x1500;
  *(vu16*)0x8020000=0xd200;
  *(vu16*)0x8040000=0x1500;
  *(vu16*)0x9e20000=data;
  *(vu16*)0x9fc0000=0x1500;
}

void cExpansion::EnableBrowser(void)
{
  for (u32 i=0;i<0x100;i+=4)
  {
    *(vu32*)(0x9000000+i)=0xffffffff;
    *(vu32*)(0x8000000+i)=0xffffffff;
  }
  *(vu32*)0x90000b0=0xffff;
  *(vu32*)0x90000b4=0x24242400;
  *(vu32*)0x90000b8=0xffffffff;
  *(vu32*)0x90000bc=0x7fffffff;
  *(vu32*)0x901fffc=0x7fffffff;
  *(vu16*)0x9240002=1;
}


void cExpansion::Block_Erase(u32 blockAdd)
{
  vu16 v1,v2;
  u32 Address;
  u32 loop;
  u32 off=0;
  if ((blockAdd>=0x1000000)&&(iId==0x227E2202))
  {
    off=0x1000000;
    *((vu16*)(FlashBase+off+0x555*2))=0xF0;
    *((vu16*)(FlashBase+off+0x1555*2))=0xF0;
  }
  else
    off=0;
  Address=blockAdd;
  *((vu16*)(FlashBase+0x555*2))=0xF0;
  *((vu16*)(FlashBase+0x1555*2))=0xF0;
  if ((blockAdd==0)||(blockAdd==0x1FC0000)||(blockAdd==0xFC0000)||(blockAdd==0x1000000))
  {
    for (loop=0;loop<0x40000;loop+=0x8000)
    {
      *((vu16*)(FlashBase+off+0x555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x555*2))=0x80;
      *((vu16*)(FlashBase+off+0x555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
      *((vu16*)(FlashBase+Address+loop))=0x30;

      *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x1555*2))=0x80;
      *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
      *((vu16*)(FlashBase+Address+loop+0x2000))=0x30;

      *((vu16*)(FlashBase+off+0x2555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x22AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x2555*2))=0x80;
      *((vu16*)(FlashBase+off+0x2555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x22AA*2))=0x55;
      *((vu16*)(FlashBase+Address+loop+0x4000))=0x30;

      *((vu16*)(FlashBase+off+0x3555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x32AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x3555*2))=0x80;
      *((vu16*)(FlashBase+off+0x3555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x32AA*2))=0x55;
      *((vu16*)(FlashBase+Address+loop+0x6000))=0x30;
      do
      {
        v1=*((vu16*)(FlashBase+Address+loop));
        v2=*((vu16*)(FlashBase+Address+loop));
      } while (v1!=v2);
      do
      {
        v1=*((vu16*)(FlashBase+Address+loop+0x2000));
        v2=*((vu16*)(FlashBase+Address+loop+0x2000));
      } while (v1!=v2);
      do
      {
        v1=*((vu16*)(FlashBase+Address+loop+0x4000));
        v2=*((vu16*)(FlashBase+Address+loop+0x4000));
      }while (v1!=v2);
      do
      {
        v1=*((vu16*)(FlashBase+Address+loop+0x6000));
        v2=*((vu16*)(FlashBase+Address+loop+0x6000));
      }while (v1!=v2);
    }
  }
  else
  {
    *((vu16*)(FlashBase+off+0x555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
    *((vu16*)(FlashBase+off+0x555*2))=0x80;
    *((vu16*)(FlashBase+off+0x555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
    *((vu16*)(FlashBase+Address))=0x30;

    *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
    *((vu16*)(FlashBase+off+0x1555*2))=0x80;
    *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
    *((vu16*)(FlashBase+Address+0x2000))=0x30;

    do
    {
      v1=*((vu16*)(FlashBase+Address));
      v2=*((vu16*)(FlashBase+Address));
    }while (v1!=v2);
    do
    {
      v1=*((vu16*)(FlashBase+Address+0x2000));
      v2=*((vu16*)(FlashBase+Address+0x2000));
    }while (v1!=v2);

    *((vu16*)(FlashBase+off+0x555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
    *((vu16*)(FlashBase+off+0x555*2))=0x80;
    *((vu16*)(FlashBase+off+0x555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
    *((vu16*)(FlashBase+Address+0x20000))=0x30;

    *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
    *((vu16*)(FlashBase+off+0x1555*2))=0x80;
    *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
    *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
    *((vu16*)(FlashBase+Address+0x2000+0x20000))=0x30;

    do
    {
      v1=*((vu16*)(FlashBase+Address+0x20000));
      v2=*((vu16*)(FlashBase+Address+0x20000));
    } while (v1!=v2);
    do
    {
      v1=*((vu16*)(FlashBase+Address+0x2000+0x20000));
      v2=*((vu16*)(FlashBase+Address+0x2000+0x20000));
    } while (v1!=v2);
  }
}

void cExpansion::WriteNorFlash(u32 address,const u8* buffer,u32 size)
{
  vu16 v1,v2;
  /*register*/ u32 loopwrite;
  vu16* buf=(vu16*)buffer;
  u32 size2,lop;
  u32 mapaddress;
  u32 j;
  v1=0;v2=1;
  u32 off=0;
  if ((address>=0x1000000)&&(iId==0x227E2202))
  {
    off=0x1000000;
  }
  else
    off=0;
  if (size>0x4000)
  {
    size2=size>>1;
    lop=2;
  }
  else
  {
    size2=size;
    lop=1;
  }
  mapaddress=address;
  for (j=0;j<lop;j++)
  {
    if (j!=0)
    {
      mapaddress+=0x4000;
      buf=(vu16*)(buffer+0x4000);
    }
    for (loopwrite=0;loopwrite<(size2>>2);loopwrite++)
    {
      *((vu16*)(FlashBase+off+0x555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x2AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x555*2))=0xA0;
      *((vu16*)(FlashBase+mapaddress+loopwrite*2))=buf[loopwrite];

      *((vu16*)(FlashBase+off+0x1555*2))=0xAA;
      *((vu16*)(FlashBase+off+0x12AA*2))=0x55;
      *((vu16*)(FlashBase+off+0x1555*2))=0xA0;
      *((vu16*)(FlashBase+mapaddress+0x2000+loopwrite*2))=buf[0x1000+loopwrite];
      do
      {
        v1=*((vu16*)(FlashBase+mapaddress+loopwrite*2));
        v2=*((vu16*)(FlashBase+mapaddress+loopwrite*2));
      }while (v1!=v2);
      do
      {
        v1=*((vu16*)(FlashBase+mapaddress+0x2000+loopwrite*2));
        v2=*((vu16*)(FlashBase+mapaddress+0x2000+loopwrite*2));
      }while (v1!=v2);
    }
  }
}

void cExpansion::WritePSRAM(u32 address,const u8* buffer,u32 size)
{
  u16* addr=(u16*)(address+_PSRAM);
  u16* pData=(u16*)buffer;
  for (u32 i=0;i<size;i+=2)
  {
    addr[i>>1]=pData[i>>1];
  }
}

void cExpansion::WriteSram(uint32 address,const u8* data,uint32 size)
{
  for (u32 i=0;i<size;i++)
    *(u8*)(address+i)=data[i];
}

void cExpansion::ReadSram(uint32 address,u8* data,uint32 size)
{
  u16* pData=(u16*)data;
  for (u32 i=0;i<size;i+=2)
  {
    pData[i>>1]=*(u8*)(address+i)+(*(u8*)(address+i+1)*0x100);
  }
}

void cExpansion::SoftReset(void)
{
  CloseNorWrite();
  SetRompage(0);
  SetRampage(16);
  SetShake(8);
}

void cExpansion::ReadNorFlashID(void)
{
  vu16 id1,id2;
  *((vu16*)(FlashBase+0x555*2))=0xAA;
  *((vu16*)(FlashBase+0x2AA*2))=0x55;
  *((vu16*)(FlashBase+0x555*2))=0x90;

  *((vu16*)(FlashBase+0x1555*2))=0xAA;
  *((vu16*)(FlashBase+0x12AA*2))=0x55;
  *((vu16*)(FlashBase+0x1555*2))=0x90;

  id1=*((vu16*)(FlashBase+0x2));
  id2=*((vu16*)(FlashBase+0x2002));
  if ((id1!=0x227E)||(id2!=0x227E)) return;

  id1=*((vu16*)(FlashBase+0xE*2));
  id2=*((vu16*)(FlashBase+0x100e*2));
  if (id1==0x2218&&id2==0x2218) //H6H6
  {
    iId=0x227E2218;
    return;
  }
  if ((id1==0x2202&&id2==0x2202)
   ||(id1==0x2202&&id2==0x2220)
   ||(id1==0x2202&&id2==0x2215)) //VZ064
  {
    iId=0x227E2202;
    return;
  }
}

void cExpansion::ChipReset(void)
{
  *((vu16*)(FlashBase))=0xF0;
  *((vu16*)(FlashBase+0x1000*2))=0xF0;
  if (iId==0x227E2202)
  {
    *((vu16*)(FlashBase+0x1000000))=0xF0 ;
    *((vu16*)(FlashBase+0x1000000+0x1000*2))=0xF0;
  }
}
