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
 *  Keyboard Module
 *
 *
 *  Emulates the keyboard by triggering the lpen input
 *  on the CRTC when mem_addr matches a sad (depressed) key.
 *
 *
 *  To do:
 *    - make keymap user reconfigurable
 */


#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

#include "keyb.h"
#include "gui.h"
#include "crtc.h"
#include "vdu.h"
#include "nanowasp.h"


SDLKey keymap[] =
  { SDLK_QUOTE,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_w,
    SDLK_x,
    SDLK_y,
    SDLK_z,
    SDLK_LEFTBRACKET,
    SDLK_BACKSLASH,
    SDLK_RIGHTBRACKET,
    SDLK_BACKQUOTE,
    SDLK_DELETE,
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_SEMICOLON,
    SDLK_EQUALS,
    SDLK_COMMA,
    SDLK_MINUS,
    SDLK_PERIOD,
    SDLK_SLASH,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_KP_ENTER,      /* LF */
    SDLK_RETURN,
    SDLK_CAPSLOCK,
    SDLK_KP_PLUS,       /* break */
    SDLK_SPACE,
    SDLK_F1,            /* 61 */
    SDLK_LCTRL,
    SDLK_F2,            /* 62 */
    SDLK_F5,            /* 65 */
    SDLK_F4,            /* 64 */
    SDLK_F3,            /* 63 */
    SDLK_F6,            /* 66 */
    SDLK_LSHIFT };



int keyb_init(void)
{
   return 0;
}


int keyb_deinit(void)
{
   return 0;
}


int keyb_reset(void)
{
   return 0;
}


void keyb_handler(int addr)
{
   int scan;

   scan = (addr >> 4) & 0x3F;

   if (debuglvl > 0)
      fprintf(debuglog, "keyb: addr = 0x%04x, scan = 0x%02x\n",
              addr, scan);

   SDL_PumpEvents();
   if (getkeystate(keymap[scan]))
      crtc_lpen(addr);
}


void keyb_checkall(void)
{
   int i;


   SDL_PumpEvents();
   if (!latchrom)
      for (i=0x3F; i>=0; i--)
         if (getkeystate(keymap[i]))
         {
            crtc_lpen(i << 4);
            break;
         }
}

