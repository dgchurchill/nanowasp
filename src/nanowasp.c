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
 *  Main nanowasp Module
 *
 *
 *  Provides init, main loop, and deinit.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include "SDL.h"


/* #include <winbase.h> */  /* for sleep, PORTME */


#include "nanowasp.h"
#include "z80.h"
#include "vdu.h"
#include "crtc.h"
#include "memmap.h"
#include "keyb.h"
#include "fdc.h"
#include "mz80.h"
#include "gui.h"



static int init(void);
static int deinit(void);
static int reset(void);


#define TITLESTRING "nanowasp v" VERSION
#define ICONSTRING  "nanowasp"

#define SPEED 3375000
#define SPEEDCORRECT 100


FILE *debuglog;
int  debuglvl = 0;


int z80_cycles = 0;



static int init(void)
{
   SDL_Surface *icon;



   /* FIXME: make debug macro, and write log to home dir */
   if (debuglvl > 0)
   {
      debuglog = fopen("nwlog.txt", "w"); 
      if (debuglog == NULL)
      {
         fprintf(stderr, "unable to write nwlog.txt\n");
         return -1;
      }

      fprintf(debuglog, "debug level = %i\n", debuglvl);
   }

   chdir(PKGDATADIR);  /* FIXME for win32.  at the moment this will
                          probably work because the directory won't exist
                          and the files will be in the current directory */

   if (SDL_Init(SDL_INIT_VIDEO) != 0)
   {
      fprintf(stderr, "init: SDL_Init failed - %s\n", SDL_GetError());
      return -1;
   }

   SDL_EventState(SDL_ACTIVEEVENT | SDL_KEYUP | SDL_KEYDOWN | SDL_MOUSEMOTION |
                  SDL_MOUSEBUTTONUP | SDL_MOUSEBUTTONDOWN | SDL_JOYAXISMOTION |
                  SDL_JOYBALLMOTION | SDL_JOYHATMOTION | SDL_JOYBUTTONUP |
                  SDL_JOYBUTTONDOWN | SDL_SYSWMEVENT | SDL_VIDEORESIZE,
                  SDL_IGNORE);

   SDL_WM_SetCaption(TITLESTRING, ICONSTRING);
   if ((icon = SDL_LoadBMP("gui/nanowasp-icon.bmp")) != NULL)
   {
      SDL_WM_SetIcon(icon, NULL);
      SDL_FreeSurface(icon);
   }

   printf("gui_init\n");
   if (gui_init() != 0)
      return -1;
   printf("z80_init\n");
   if (z80_init() != 0)
      return -1;
   printf("memmap_init\n");
   if (memmap_init() != 0)
      return -1;
   printf("crtc_init\n");
   if (crtc_init() != 0)
      return -1;
   printf("keyb_init\n");
   if (keyb_init() != 0)
      return -1;
   printf("vdu_init\n");
   if (vdu_init() != 0)
      return -1;
   printf("fdc_init\n");
   if (fdc_init() != 0)
      return -1;

   return 0;
}



/* depends on short circuit evaluation */
static int deinit(void)
{
   int res;

   if (debuglvl > 0)
      fclose(debuglog);


   res = fdc_deinit() ||
         vdu_deinit() ||
         keyb_deinit() ||
         crtc_deinit() ||
         memmap_deinit() ||
         z80_deinit() ||
         gui_deinit();

   SDL_Quit();

   return res;
}



static int reset(void)
{
   return z80_reset() ||
          memmap_reset() ||
          crtc_reset() ||
          keyb_reset() ||
          vdu_reset() ||
          fdc_reset();
}


int main(int argc, char *argv[])
{
   int opt;
   int done = 0;
   time_t starttime;
   int delay = 0, speed = 0;


   while ((opt = getopt(argc, argv, "d")) != -1)
   {
      switch(opt)
      {
         case 'd':  debuglvl++;  break;
         default:   exit(1);     break;
      }
   }

   if (init() != 0)
   {
      fprintf(stderr, "Fatal error during initialisation.  Exiting...\n");
      exit(1);
   }

   reset();

   /* main loop */
   starttime = time(NULL)-1;
   while (!done)
   {
      mz80exec(100000);
      gui_redraw();
      usleep(delay);


      z80_cycles += mz80GetElapsedTicks(1);  /* FIXME: overflow */
      speed = z80_cycles / (time(NULL) - starttime);


      if (speed > SPEED)  /* FIXME */
         delay += SPEEDCORRECT;
      else
      {
         delay -= SPEEDCORRECT;
         if (delay < 0)
            delay = 0;
      }

      SDL_PumpEvents();
      if (getkeystate(SDLK_F12) || SDL_QuitRequested())
         done = 1;
      if (getkeystate(SDLK_F11))
         reset();
   }


   if (deinit() != 0)
   {
      fprintf(stderr, "Error while de-initialising.  Hrrmm... Oh well.\n");
      exit(1);
   }


   printf("last speed: %i kHz  last delay: %i usec\n",
          (int)(speed / 1000), delay);

   return 0;
}

