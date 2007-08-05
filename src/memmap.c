/*  nanowasp, an emulator for the microbee 128k
 *  Copyright (C) 2000-2003  David G. Churchill <froods@alphalink.com.au>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *
 *  Memory mapper module (MW168)
 *
 *
 *  Emulates the memory bank switching and video active signal.
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memmap.h"
#include "nanowasp.h"
#include "z80.h"
#include "vdu.h"
#include "mz80.h"


#define BANKSIZE  32768
#define ROMSIZE   16384

#define MAXMEMHANDLERS  5


static unsigned int roundup(unsigned int n);  /* rounds up to a power of 2 */
static int loadrom(char *name, char *dest);   /* load a rom image */
static void switchin(int newmode);
static void switchout(int oldmode);
static void romwrite(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s);
static void dupwrite(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s);


static struct MemoryWriteByte z80_mem_w[MAXMEMHANDLERS] =
   { { -1, -1, NULL, NULL } };

static struct MemoryReadByte z80_mem_r[MAXMEMHANDLERS] =
   { { -1, -1, NULL, NULL } };



static char rom1[ROMSIZE], rom2[ROMSIZE], rom3[ROMSIZE];
static char bank0_0[BANKSIZE], bank0_1[BANKSIZE],
            bank1_0[BANKSIZE], bank1_1[BANKSIZE];

static int mode;




/* rounds up to a power of 2 */
static unsigned int roundup(unsigned int n)
{
   int bits_set = 0, high_bit = 0, i, n2 = n;



   for (i=0; i<(sizeof(unsigned int)*8); i++)
   {
      if (n2 & 1)
      {
         high_bit = i;
         bits_set++;
      }
      n2 >>= 1;
   }


   if (bits_set > 1)
      return 1 << (high_bit+1);
   else
      return n;
}



/* load a rom image */
static int loadrom(char *name, char *dest)
{
   FILE *romfp;
   unsigned int i;
   long len;


   if (dest == NULL)
      return -1;

   memset(dest, 0, ROMSIZE);


   romfp = fopen(name, "rb");
   if (romfp == NULL)
      return -1;

   fseek(romfp, 0, SEEK_END);   /* get size of rom */
   len = ftell(romfp);
   fseek(romfp, 0, SEEK_SET);
   if (len > ROMSIZE)
      len = ROMSIZE;

   if (len == 0)
      return -1;

   fread(dest, 1, len, romfp);
   fclose(romfp);
   len = roundup(len);
   for (i=len; i<ROMSIZE; i++)
      rom1[i] = rom1[i % len];

   return 0;
}



int memmap_init(void)
{
   struct mz80context z80;


   memset(bank0_0, 0, BANKSIZE);
   memset(bank0_1, 0, BANKSIZE);
   memset(bank1_0, 0, BANKSIZE);
   memset(bank1_1, 0, BANKSIZE);


   if (loadrom("rom1.bin", rom1) != 0)
   {
      fprintf(stderr, "Unable to load rom1.bin (required to boot).\n");
      return -1;
   }

   loadrom("rom2.bin", rom2);
   loadrom("rom3.bin", rom3);


   mode = -1;

   mz80GetContext(&z80);
   z80.z80MemRead = z80_mem_r;
   z80.z80MemWrite = z80_mem_w;
   mz80SetContext(&z80);

   return 0;
}


int memmap_deinit(void)
{
   return 0;
}


int memmap_reset(void)
{
   memmap_mode(0, 0, NULL);

   return 0;
}


void memmap_mode(UINT16 port, UINT8 data, struct z80PortWrite *port_struct)
{
   if (debuglvl > 0)
      fprintf(debuglog, "memmap_mode set = 0x%02x\n", data);


   if (mode != -1)
      switchout(mode);

   switchin(mode = (data & 0x1F));
}



static void switchin(int newmode)
{
   int rindex = 0, windex = 0;
   char *bank;


   /* do lower 32k ram, no handler needed */
   switch (newmode & 0x07)
   {
      case 0:  bank = bank0_1; break;   /* bank 0 upper */
      case 1:  bank = bank1_1; break;   /* bank 1 upper */
      case 2:  bank = bank0_0; break;   /* bank 0 lower */
      case 3:  bank = bank1_0; break;   /* bank 1 lower */
      case 4:  bank = bank0_0; break;   /* bank 0 lower */
      case 5:  bank = bank1_0; break;   /* bank 1 lower */
      case 6:  bank = bank0_1; break;   /* bank 0 upper */
      case 7:  bank = bank1_1; break;   /* bank 1 upper */
   }
   memcpy(z80mem, bank, BANKSIZE);



   /* insert video memory handler if enabled
        Must appear early in the list of handlers so it overrides
        anthing that overlaps it.  The handler that gets called
        is the first one with matching range, as a result of the mz80
        implementation.  This might change in the future! */

   if (!(newmode & 0x08))  /* b3 = vidmem disable */
   {
      /* vidmem enabled */

      z80_mem_r[rindex].memoryCall = vdu_vidmem_r;
      z80_mem_w[windex].memoryCall = vdu_vidmem_w;
      z80_mem_r[rindex].pUserArea  = NULL;
      z80_mem_w[windex].pUserArea  = NULL;


      if (newmode & 0x10)  /* b4 = vidmem location */
      {
         z80_mem_r[rindex].lowAddr =  0x8000;
         z80_mem_w[windex].lowAddr =  0x8000;
         z80_mem_r[rindex].highAddr = 0x8FFF;
         z80_mem_w[windex].highAddr = 0x8FFF;
      }
      else
      {
         z80_mem_r[rindex].lowAddr =  0xF000;
         z80_mem_w[windex].lowAddr =  0xF000;
         z80_mem_r[rindex].highAddr = 0xFFFF;
         z80_mem_w[windex].highAddr = 0xFFFF;
      }

      rindex++;
      windex++;
   }



   /* do upper 32k, roms need handler */
   if (newmode & 0x04)     /* b2 = rom disable */
   {
      memcpy(z80mem+BANKSIZE, bank0_1, BANKSIZE);

      if ((newmode & 0x07) == 6)
      {
         /* this bank is mapped into upper and lower, so we need
            to echo any writes to one half to the other */
         z80_mem_w[windex].lowAddr = 0x0000;
         z80_mem_w[windex].highAddr = 0xFFFF;
         z80_mem_w[windex].memoryCall = dupwrite;
         z80_mem_w[windex].pUserArea = NULL;
         windex++;
      }

   }
   else
   {
      /* roms enabled */

      memcpy(z80mem+BANKSIZE, rom1, ROMSIZE);

      if (newmode & 0x20)   /* b5 = rom select */
         memcpy(z80mem+BANKSIZE+ROMSIZE, rom3, ROMSIZE);
      else
         memcpy(z80mem+BANKSIZE+ROMSIZE, rom2, ROMSIZE);


      z80_mem_w[windex].lowAddr = 0x8000;
      z80_mem_w[windex].highAddr = 0xFFFF;
      z80_mem_w[windex].memoryCall = romwrite;
      z80_mem_w[windex].pUserArea = NULL;
      windex++;
   }


   /* copy video memory into main memory
      done last so it overwrites anything already there */
   if (!(newmode & 0x08))  /* b3 = vidmem disable */
   {
      if (newmode & 0x10)  /* b4 = vidmem location */
      {
         memcpy(z80mem+0x8000, videoram, 0x800);
         memcpy(z80mem+0x8800, pcgram, 0x800);
      }
      else
      {
         memcpy(z80mem+0xF000, videoram, 0x800);
         memcpy(z80mem+0xF800, pcgram, 0x800);
      }
   }


   z80_mem_r[rindex].lowAddr = -1;
   z80_mem_r[rindex].highAddr = -1;
   z80_mem_r[rindex].memoryCall = NULL;
   z80_mem_r[rindex].pUserArea = NULL;

   z80_mem_w[windex].lowAddr = -1;
   z80_mem_w[windex].highAddr = -1;
   z80_mem_w[windex].memoryCall = NULL;
   z80_mem_w[windex].pUserArea = NULL;
}



static void switchout(int oldmode)
{
   char *bank;

   switch (oldmode & 0x07)
   {
      case 0:  bank = bank0_1; break;   /* bank 0 upper */
      case 1:  bank = bank1_1; break;   /* bank 1 upper */
      case 2:  bank = bank0_0; break;   /* bank 0 lower */
      case 3:  bank = bank1_0; break;   /* bank 1 lower */
      case 4:  bank = bank0_0; break;   /* bank 0 lower */
      case 5:  bank = bank1_0; break;   /* bank 1 lower */
      case 6:  bank = bank0_1; break;   /* bank 0 upper */
      case 7:  bank = bank1_1; break;   /* bank 1 upper */
   }
   memcpy(bank, z80mem, BANKSIZE);


   if (!(oldmode & 0x08))  /* b3 = vidmem disable */
   {
      if (oldmode & 0x10)  /* b4 = vidmem location */
      {
         memcpy(videoram, z80mem+0x8000, 0x800);
         memcpy(pcgram, z80mem+0x8800, 0x800);

         if ((oldmode & 0x04) &&       /* b2 = rom disable */
             ((oldmode & 0x07) != 6))
            memcpy(bank0_1+0x1000, z80mem+BANKSIZE+0x1000, BANKSIZE-0x1000);
      }
      else
      {
         memcpy(videoram, z80mem+0xF000, 0x800);
         memcpy(pcgram, z80mem+0xF800, 0x800);

         if ((oldmode & 0x04) &&       /* b2 = rom disable */
             ((oldmode & 0x07) != 6))
            memcpy(bank0_1, z80mem+BANKSIZE, BANKSIZE-0x1000);
      }
   }
   else
      if ((oldmode & 0x04) &&       /* b2 = rom disable */
          ((oldmode & 0x07) != 6))
         memcpy(bank0_1, z80mem+BANKSIZE, BANKSIZE);
}



static void romwrite(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s)
{
   /* this space intentionally left blank */

   /* oh, you want a reason.  this routine _writes_ to a _rom_. */
}


/* duplicates writes to one half of ram to the other half */
static void dupwrite(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s)
{
   z80mem[addr & 0x7FFF] = data;
   z80mem[addr | 0x8000] = data;
}

