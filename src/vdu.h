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
 *   VDU Header
 *
 */


#ifndef HEADER_VDU_H
#define HEADER_VDU_H

#include "mz80.h"


#define VIDEORAMSIZE  2048
#define CHARROMSIZE   4096
#define PCGRAMSIZE    2048


extern char videoram[], charrom[], pcgram[];
extern char latchrom;


int vdu_init(void);
int vdu_deinit(void);
int vdu_reset(void);

UINT8 vdu_vidmem_r(UINT32 addr, struct MemoryReadByte *mem_s);
void  vdu_vidmem_w(UINT32 addr, UINT8 data, struct MemoryWriteByte *mem_s);

void vdu_latchrom(UINT16 port, UINT8 data, struct z80PortWrite *port_s);

#endif /* HEADER_VDU_H */

