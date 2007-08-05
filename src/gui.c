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
 *  GUI Module
 *
 *
 *  Provides the graphical interface.
 *
 *  PORTME The majority of this module relies on Allegro
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

#include "gui.h"
#include "crtc.h"


static SDL_Surface *screen;
static Uint8 *keystate;


int gui_init(void)
{
   /* init graphics mode */
   screen = SDL_SetVideoMode(VIDWIDTH, VIDHEIGHT, 16, 0);
   if (screen == NULL)
   {
      fprintf(stderr, "gui_init: SDL_SetVideoMode failed - %s\n",
              SDL_GetError());
      return -1;
   }

   keystate = SDL_GetKeyState(NULL);
   if (keystate == NULL)
   {
      fprintf(stderr, "init: SDL_GetKeyState returned NULL\n");
      return -1;
   }

   return 0;
}


int gui_deinit(void)
{
   return 0;
}


int gui_reset(void)
{
   return 0;
}



void gui_redraw(void)
{
   crtc_redraw();
   SDL_UpdateRect(screen, 0, 0, 0, 0);
}


inline void putpixel(SDL_Surface *surf, int x, int y, Uint32 col)
{
   Uint8 *p = (Uint8 *)surf->pixels + y * surf->pitch + x * 2;
   if ( (x < surf->w) && (y < surf->h) )
      *(Uint16 *)p = col;
}


int getkeystate(SDLKey key)
{
   static int last_capslock = 0;
   static int capslock_count = 3;

   if (key == SDLK_CAPSLOCK)
   {
      if (keystate[SDLK_CAPSLOCK] != last_capslock)
      {
         if (--capslock_count == 0)
         {
            last_capslock = keystate[SDLK_CAPSLOCK];
            capslock_count = 3;
         }
         return 1;
      }
      return 0;
   }

   if ((key == SDLK_LSHIFT) || (key == SDLK_RSHIFT))
      return keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT];
   if ((key == SDLK_LCTRL) || (key == SDLK_RCTRL))
      return keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL];

   return keystate[key];
}

