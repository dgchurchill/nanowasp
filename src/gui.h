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
 *  GUI Header
 *
 *
 */


#ifndef HEADER_GUI_H
#define HEADER_GUI_H


#define VIDWIDTH 800
#define VIDHEIGHT 450

#define VDUPOSX   0
#define VDUPOSY   0

int gui_init(void);
int gui_deinit(void);
int gui_reset(void);


inline void putpixel(SDL_Surface *surf, int x, int y, Uint32 col);
inline int getkeystate(SDLKey key);
void gui_redraw(void);


#endif /* HEADER_GUI_H */

