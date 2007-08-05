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
 *   FDC Header
 *
 */

#ifndef HEADER_FDC_H
#define HEADER_FDC_H

#include "mz80.h"



#define FDC_NUMDRIVES  2



int fdc_init(void);
int fdc_deinit(void);
int fdc_reset(void);


int fdc_loaddisk(const char *name, int drive);
void fdc_unloaddisk(int drive);


UINT16 fdc_status(UINT16 port, struct z80PortRead *port_s);
void fdc_cmd(UINT16 port, UINT8 data, struct z80PortWrite *port_s);

UINT16 fdc_data_r(UINT16 port, struct z80PortRead *port_s);
void fdc_data_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s);
UINT16 fdc_track_r(UINT16 port, struct z80PortRead *port_s);
void fdc_track_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s);
UINT16 fdc_sect_r(UINT16 port, struct z80PortRead *port_s);
void fdc_sect_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s);

UINT16 fdc_ext_r(UINT16 port, struct z80PortRead *port_s);
void fdc_ext_w(UINT16 port, UINT8 data, struct z80PortWrite *port_s);


#endif  /* HEADER_FDC_H */

