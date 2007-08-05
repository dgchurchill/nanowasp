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
 *  z80 Module
 *
 *
 *  Provides an init/deinit and an interface to the MZ80 routines.
 *
 */


#include <stdio.h>
#include <stdlib.h>

#include "z80.h"
#include "nanowasp.h"
#include "mz80.h"
#include "crtc.h"
#include "memmap.h"
#include "vdu.h"
#include "fdc.h"




UINT16 z80_unhandled_r(UINT16 port, struct z80PortRead *port_s);
void z80_unhandled_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s);



static struct z80PortRead z80_ports_r[] =
{
   { 0x0C, 0x0C, crtc_status, NULL },
   { 0x0E, 0x0E, crtc_status, NULL },
   { 0x1C, 0x1C, crtc_status, NULL },
   { 0x1E, 0x1E, crtc_status, NULL },

   { 0x0D, 0x0D, crtc_data_r, NULL },
   { 0x0F, 0x0F, crtc_data_r, NULL },
   { 0x1D, 0x1D, crtc_data_r, NULL },
   { 0x1F, 0x1F, crtc_data_r, NULL },

   { 0x40, 0x40, fdc_status, NULL },
   { 0x41, 0x41, fdc_track_r, NULL },
   { 0x42, 0x42, fdc_sect_r, NULL },
   { 0x43, 0x43, fdc_data_r, NULL },
   { 0x44, 0x44, fdc_status, NULL },
   { 0x45, 0x45, fdc_track_r, NULL },
   { 0x46, 0x46, fdc_sect_r, NULL },
   { 0x47, 0x47, fdc_data_r, NULL },
   { 0x48, 0x4B, fdc_ext_r, NULL },

   { 0x00, 0xFF, z80_unhandled_r, NULL },

   { -1, -1, NULL, NULL }
};



static struct z80PortWrite z80_ports_w[] =
{
   { 0x0B, 0x0B, vdu_latchrom, NULL },

   { 0x0C, 0x0C, crtc_address, NULL },
   { 0x0E, 0x0E, crtc_address, NULL },
   { 0x1C, 0x1C, crtc_address, NULL },
   { 0x1E, 0x1E, crtc_address, NULL },

   { 0x0D, 0x0D, crtc_data_w, NULL },
   { 0x0F, 0x0F, crtc_data_w, NULL },
   { 0x1D, 0x1D, crtc_data_w, NULL },
   { 0x1F, 0x1F, crtc_data_w, NULL },

   { 0x40, 0x40, fdc_cmd, NULL },
   { 0x41, 0x41, fdc_track_w, NULL },
   { 0x42, 0x42, fdc_sect_w, NULL },
   { 0x43, 0x43, fdc_data_w, NULL },
   { 0x44, 0x44, fdc_cmd, NULL },
   { 0x45, 0x45, fdc_track_w, NULL },
   { 0x46, 0x46, fdc_sect_w, NULL },
   { 0x47, 0x47, fdc_data_w, NULL },
   { 0x48, 0x4B, fdc_ext_w, NULL },

   { 0x50, 0x57, memmap_mode, NULL },

   { 0x00, 0xFF, z80_unhandled_w, NULL },

   { -1, -1, NULL, NULL }
};


char z80mem[Z80MEMSIZE];



int z80_init(void)
{
   struct mz80context z80;


   z80.z80Base = z80mem;

   z80.z80IoRead = z80_ports_r;
   z80.z80IoWrite = z80_ports_w;

   z80.z80MemRead = NULL;
   z80.z80MemWrite = NULL;

   /* put interrupt setup stuff here */

   mz80SetContext(&z80);

   return 0;
}



int z80_deinit(void)
{
   return 0;
}



int z80_reset(void)
{
   struct mz80context z80;

   mz80reset();
   mz80GetContext(&z80);
   z80.z80pc = 0x8000;
   mz80SetContext(&z80);

   return 0;
}



int z80_getpc(void)
{
   struct mz80context z80;

   mz80GetContext(&z80);
   return z80.z80pc;
}



UINT16 z80_unhandled_r(UINT16 port, struct z80PortRead *port_s)
{
/*   fprintf(stderr, "unk read: %02X\n", port); */
   return 0;
}


void z80_unhandled_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s)
{
/*   fprintf(stderr, "unk write: %02X = %02X\n", port, data); */
}

