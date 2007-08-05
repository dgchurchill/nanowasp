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
 *  6545 CRTC Module -- Fast Version
 *
 *
 *  *Doesn't* emulate the _signals_ of the 6545 CRTC IC.
 *
 *  This module does most of the graphics stuff, and isn't
 *  very general at all.  For a general (slow), reasonably
 *  accurate emulation use the crtc-6545 module.  Hopefully
 *  most of the emulated system code will run fine under this.
 *
 *
 *  To do:
 *    - Cursor blinking.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "SDL.h"

#include "crtc.h"
#include "keyb.h"
#include "vdu.h"
#include "gui.h"
#include "nanowasp.h"
#include "mz80.h"



#define CCLOCK_RATE  2     /* char clocks / z80 clocks */



static void calc_vblank(void);



int disp_start;

static int htot, hdisp;
static int vtot, vtot_adj, vdisp;
static int scans_per_row;
static int cur_start, cur_end, cur_blink, cur_pos;
static int lpen_valid, lpen;
static int reg;
static int vblank_freq,    /* cycles b/w vblank starts */
           vblank_len;     /* cycles vblank is active for */
static int mem_addr;


static SDL_Surface *vdu;
static SDL_Rect rect_vdu = { 0, 0, VDUWIDTH, VDUHEIGHT };
static SDL_Surface *screen;
static SDL_Rect rect_screen = { VDUPOSX, VDUPOSY, VDUWIDTH, VDUHEIGHT };
static int redraw;

static Uint32 bgcolour, fgcolour;



int crtc_init(void)
{
   SDL_PixelFormat *spf;

   screen = SDL_GetVideoSurface();
   spf  = screen->format;
   vdu = SDL_CreateRGBSurface(0, VDUWIDTH, VDUHEIGHT, spf->BitsPerPixel,
             spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask);
   if (vdu == NULL)
   {
      fprintf(stderr, "crtc_init: SDL_CreateRGBSurface failed - %s\n", SDL_GetError());
      return -1;
   }

   fgcolour = SDL_MapRGB(spf, FGR, FGG, FGB);
   bgcolour = SDL_MapRGB(spf, BGR, BGG, BGB);

   SDL_FillRect(vdu, &rect_vdu, bgcolour);

   return 0;
}


int crtc_deinit(void)
{
   SDL_FreeSurface(vdu);

   return 0;
}


int crtc_reset(void)
{
   reg = 0;

   return 0;
}



UINT16 crtc_status(UINT16 port, struct z80PortRead *port_s)
{
   int status = 0x80;    /* always set update strobe */



   if (!lpen_valid)
      keyb_checkall();

   if (lpen_valid)     /* NB: this is not an else because */
      status |= 0x40;  /* keyb_checkall might set lpen_valid */

   if (z80_cycles % vblank_freq < vblank_len)
      status |= 0x20;


   if (debuglvl > 0)
      fprintf(debuglog, "crtc_status read = 0x%02x\n", status);


   return status;
}



void crtc_lpen(int addr)
{
   if (!lpen_valid)
   {
      lpen_valid = 1;
      lpen = addr;
      if (debuglvl > 0)
         fprintf(debuglog, "crtc_lpen addr = 0x%04x\n", addr);
   }
}



void crtc_address(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   reg = data & 0x1F;

/*   if (debuglvl > 0)
      fprintf(debuglog, "crtc_address set = 0x%02x\n", data); */
}



UINT16 crtc_data_r(UINT16 port, struct z80PortRead *port_s)
{
/*   if (debuglvl > 0)
      fprintf(debuglog, "crtc_data read\n"); */


   switch (reg)
   {
      case CRTC_CUR_POS_H:
         return (cur_pos >> 8) & 0x3F;
         break;
      case CRTC_CUR_POS_L:
         return cur_pos & 0xFF;
         break;

      case CRTC_LPEN_H:
         if (debuglvl > 0)
            fprintf(debuglog, "crtc_lpen_h: lpen = 0x%04x\n", lpen);
         lpen_valid = 0;
         return (lpen >> 8) & 0x3F;
         break;
      case CRTC_LPEN_L:
         lpen_valid = 0;
         if (debuglvl > 0)
            fprintf(debuglog, "crtc_lpen_l: lpen = 0x%04x\n", lpen);
         return lpen & 0xFF;
         break;

      default:
         return 0xFFFF;
   }

}



void crtc_data_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
   int old_curpos;

/*   if (debuglvl > 0)
      fprintf(debuglog, "crtc_data set = 0x%02x\n", data); */


   switch (reg)
   {
      case CRTC_HTOT:
         htot = (data & 0xFF) + 1;
         calc_vblank();
         break;
      case CRTC_HDISP:
         hdisp = data & 0xFF;
         SDL_FillRect(vdu, &rect_vdu, bgcolour);
         break;


      case CRTC_VTOT:
         vtot = (data & 0x7F) + 1;
         calc_vblank();
         break;
      case CRTC_VTOT_ADJ:
         vtot_adj = data & 0x1F;
         calc_vblank();
         break;
      case CRTC_VDISP:
         vdisp = data & 0x7F;
         SDL_FillRect(vdu, &rect_vdu, bgcolour);
         break;


      case CRTC_SCANLINES:
         scans_per_row = (data & 0x1F) + 1;
         calc_vblank();
         SDL_FillRect(vdu, &rect_vdu, bgcolour);
         break;


      case CRTC_CUR_START:
         cur_start = data & 0x1F;
         cur_blink = (data >> 5) & 0x03;
         crtc_redraw_char(cur_pos);
         break;
      case CRTC_CUR_END:
         cur_end = data & 0x1F;
         crtc_redraw_char(cur_pos);
         break;


      case CRTC_DISP_START_H:
         disp_start &= 0xFF;
         disp_start |= (data & 0x3F) << 8;
         break;
      case CRTC_DISP_START_L:
         disp_start &= 0x3F00;
         disp_start |= data & 0xFF;
         break;


      case CRTC_CUR_POS_H:
         old_curpos = cur_pos;
         cur_pos &= 0xFF;
         cur_pos |= (data & 0x3F) << 8;
         crtc_redraw_char(old_curpos);
         crtc_redraw_char(cur_pos);
         break;
      case CRTC_CUR_POS_L:
         old_curpos = cur_pos;
         cur_pos &= 0x3F00;
         cur_pos |= data & 0xFF;
         crtc_redraw_char(old_curpos);
         crtc_redraw_char(cur_pos);
         break;


      case CRTC_SETADDR_H:
         mem_addr =
            (mem_addr & 0x00FF) | ((data & 0x3F) << 8);
         break;
      case CRTC_SETADDR_L:
         mem_addr =
            (mem_addr & 0x3F00) | (data & 0xFF);
         break;
      case CRTC_DOSETADDR:
         if (debuglvl > 0)
            fprintf(debuglog, "crtc_dosetaddr: mem_addr = 0x%04x\n",
                    mem_addr);
         keyb_handler(mem_addr);
         break;
   }
}



static void calc_vblank(void)
{
   vblank_freq = htot * (vtot * scans_per_row + vtot_adj) / CCLOCK_RATE;
   vblank_len = htot * ((vtot - vdisp) * scans_per_row + vtot_adj) /
                CCLOCK_RATE;

   if (vblank_freq == 0)
      vblank_freq = 1;
}


void crtc_redraw_char(int addr)
{
   int i, j, x, y, offset, maddr;
   char ch, cursor;
   char *data;
   SDL_Rect src, dst;


   if (hdisp == 0)
      return;

   offset = (addr & 0x07FF) - (disp_start & 0x07FF);  /* ofs from disp_start */
   if (offset < 0)
      offset += 0x0800;
   x = offset % hdisp;
   y = offset / hdisp;
   maddr = (disp_start + offset) & 0x3FFF;

   ch = videoram[maddr & 0x07FF];
   if (ch & 0x80)      /* selects pcg ram */
      data = pcgram + 16 * (ch & 0x7F);
   else
      data = charrom + 16 * (ch & 0x7F) +
             ((maddr & 0x2000) >> 2);

   if ( (x >= hdisp) || (y >= vdisp) )
      return;

   SDL_LockSurface(vdu);
   for (i=0; i<scans_per_row; i++)
   {
      if ((maddr == cur_pos) &&
          (i >= cur_start) && (i <= cur_end))
         cursor = 0xFF;
      else
         cursor = 0x00;

      ch = *(data++) ^ cursor;

      for (j=0; j<8; j++)
      {
         if (ch & 0x80)
            putpixel(vdu, x*8+j, y*scans_per_row+i, fgcolour);
         else
            putpixel(vdu, x*8+j, y*scans_per_row+i, bgcolour);

         ch <<= 1;
      }
   }
   SDL_UnlockSurface(vdu);

   /* do sub surfaces ?!? */
   src.x = x*8;  src.y = y*scans_per_row;  src.w = 8;   src.h = scans_per_row;
   dst.x = VDUPOSX + src.x;  dst.y = VDUPOSY + src.y;
   SDL_BlitSurface(vdu, &src, screen, &dst);
}


void crtc_set_redraw(void)
{
   redraw = 1;
}


void crtc_redraw(void)
{
   int i, j, k, l;
   int maddr;
   char ch, cursor;
   char *data;


   if (!redraw)
      return;

   redraw = 0;


   SDL_LockSurface(vdu);
   maddr = disp_start;

   for (i=0; i<vdisp; i++)
      for (j=0; j<hdisp; j++)
      {
         ch = videoram[maddr & 0x07FF];
         if (ch & 0x80)      /* selects pcg ram */
            data = pcgram + 16 * (ch & 0x7F);
         else
            data = charrom + 16 * (ch & 0x7F) +
                   ((maddr & 0x2000) >> 2);

         for (k=0; k<scans_per_row; k++)
         {
            if ((maddr == cur_pos) &&
                (k >= cur_start) && (k <= cur_end))
               cursor = 0xFF;
            else
               cursor = 0x00;

            ch = *(data++) ^ cursor;

            for (l=0; l<8; l++)
            {
               if (ch & 0x80)
                  putpixel(vdu, j*8+l, i*scans_per_row+k, fgcolour);
               else
                  putpixel(vdu, j*8+l, i*scans_per_row+k, bgcolour);

               ch <<= 1;
            }
         }

         maddr = (maddr + 1) & 0x3FFF;
      }

   SDL_UnlockSurface(vdu);

   SDL_BlitSurface(vdu, &rect_vdu, screen, &rect_screen);
}

