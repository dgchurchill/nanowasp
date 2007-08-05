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
 *   6545 CRTC Header
 *
 */


#ifndef HEADER_CRTC_H
#define HEADER_CRTC_H

#include "mz80.h"


#define CRTC_HTOT           0
#define CRTC_HDISP          1
#define CRTC_HSYNC_POS      2
#define CRTC_SYNC_WIDTH     3
#define CRTC_VTOT           4
#define CRTC_VTOT_ADJ       5
#define CRTC_VDISP          6
#define CRTC_VSYNC_POS      7
#define CRTC_MODE           8
#define CRTC_SCANLINES      9
#define CRTC_CUR_START      10
#define CRTC_CUR_END        11
#define CRTC_DISP_START_H   12
#define CRTC_DISP_START_L   13
#define CRTC_CUR_POS_H      14
#define CRTC_CUR_POS_L      15
#define CRTC_LPEN_H         16
#define CRTC_LPEN_L         17
#define CRTC_SETADDR_H      18
#define CRTC_SETADDR_L      19

#define CRTC_DOSETADDR      31


#define VDUWIDTH  640
#define VDUHEIGHT 272

#define FGR  238
#define FGG  166
#define FGB  0

#define BGR  0
#define BGG  0
#define BGB  0



extern int disp_start;


int crtc_init(void);
int crtc_deinit(void);
int crtc_reset(void);


void crtc_redraw(void);
void crtc_set_redraw(void);
void crtc_redraw_char(int addr);

void crtc_lpen(int addr);

UINT16 crtc_status(UINT16 port, struct z80PortRead *port_struct);
void   crtc_address(UINT16 port, UINT8 data, struct z80PortWrite *port_struct);
UINT16 crtc_data_r(UINT16 port, struct z80PortRead *port_struct);
void   crtc_data_w(UINT16 port, UINT8 data, struct z80PortWrite *port_struct);


#endif     /* HEADER_CRTC_H */

