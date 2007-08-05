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
 *  VDU Module
 *
 *
 *  Emulates the remainder of graphics hardware (char rom, pcg ram, etc).
 *
 *
 */


#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "vdu.h"
#include "crtc.h"
#include "z80.h"
#include "nanowasp.h"
#include "mz80.h"



char videoram[VIDEORAMSIZE],
     charrom[CHARROMSIZE],
     pcgram[PCGRAMSIZE];


char latchrom;



int vdu_init(void)
{
   FILE *fp;


   memset(videoram, 0, VIDEORAMSIZE);
   memset(pcgram, 0, PCGRAMSIZE);

   /* load character rom */
   fp = fopen("charrom.bin", "rb");
   if (fp == NULL)
   {
      fprintf(stderr, "vdu_init: unable to read charrom.bin\n");
      return -1;
   }

   if (fread(charrom, 1, CHARROMSIZE, fp) != CHARROMSIZE)
   {
      fprintf(stderr, "vdu_init: charrom.bin must be %i bytes\n", CHARROMSIZE);
      return -1;
   }

   fclose(fp);


   return 0;
}


int vdu_deinit(void)
{
   return 0;
}


int vdu_reset(void)
{
   latchrom = 0;

   return 0;
}




UINT8 vdu_vidmem_r(UINT32 addr, struct MemoryReadByte *mem_s)
{
/*   if (debuglvl > 0)
      fprintf(debuglog, "vdu_vidmem_r at 0x%04x\n", (unsigned int)addr); */


   if (addr & 0x0800)
      return pcgram[addr & 0x07FF];
   else
   {
      if (!latchrom)
         return videoram[addr & 0x07FF];
      else
         return charrom[((disp_start & 0x2000) >> 2) +  /* FIXME this will not work if disp_start is set to just before 0x2000, and the memory addresses would overlap 0x2000 */
                        (addr & 0x07FF)];
   }
}


void  vdu_vidmem_w(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s)
{
/*   if (debuglvl > 0)
      fprintf(debuglog, "vdu_vidmem_w at 0x%04x = 0x%02x\n",
              (unsigned int)addr, data); */


   if (addr & 0x0800)
   {
      pcgram[addr & 0x07FF] = data;
      z80mem[addr & 0xFFFF] = data;
      crtc_set_redraw();
   }
   else
      if (!latchrom)
      {
         videoram[addr & 0x07FF] = data;
         z80mem[addr & 0xFFFF] = data;
         crtc_redraw_char(addr);
      }
}



void vdu_latchrom(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   latchrom = data & 0x01;
}

