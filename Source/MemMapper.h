/*  Nanowasp - A Microbee emulator
 *  Copyright (C) 2000-2007 David G. Churchill
 *
 *  This file is part of Nanowasp.
 *
 *  Nanowasp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nanowasp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEMMAPPER_H
#define MEMMAPPER_H

#include "PortDevice.h"

class Microbee;
class Z80CPU;
class RAM;
class ROM;
class CRTCMemory;


/*! \brief Emulates the memory mapping hardware for the 128k disk system
 *
 *  The address space for the 128k disk system is divided into two blocks
 *  of 32k.  The lower 32k holds a single 32k bank of RAM selected from
 *  the four banks (32k x 4 = 128k).  The upper 32k holds either bank 1 of
 *  the RAM, or the ROMs.  ROM 1 appears as the lower 16k of the upper block,
 *  while the upper 16k can be selected between ROMs 2 and 3.  Additionally,
 *  the graphics memory can be enabled, and appears in the upper block at either
 *  0x8000 or 0xF000.  If the graphics memory is enabled then it is accessed
 *  in preference to whatever else may be mapped in the same space.
 */
class MemMapper : public PortDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    MemMapper(Microbee &mbee_, const TiXmlElement &config_);

    virtual void LateInit();


    virtual void Reset();

    virtual void PortWrite(word addr, byte val);
    virtual byte PortRead(word addr);


private:
    Microbee &mbee;
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config

    Z80CPU *z80; //!< Connection to Z80CPU, for RegMemHandler(), RegPortHandler()
    RAM *bank0;  //!< Connection to RAM Bank 0
    RAM *bank1;  //!< Connection to RAM Bank 1
    RAM *bank2;  //!< Connection to RAM Bank 2
    RAM *bank3;  //!< Connection to RAM Bank 3
    ROM *rom1;   //!< Connection to ROM 1
    ROM *rom2;   //!< Connection to ROM 2
    ROM *rom3;   //!< Connection to ROM 3
    CRTCMemory *crtcmem;  //!< Connection to CRTCMemory


    static const byte cBank = 0x07;
    static const byte cROMDisable = 0x04;
    static const byte cVideoRAMDisable = 0x08;
    static const byte cVideoRAMLoc = 0x10;
    static const byte cROMSelect = 0x20;

    static const word cLowBlock = 0x0000;
    static const word cHighBlock = 0x8000;
    static const word cHighBlockA = cHighBlock;
    static const word cHighBlockB = cHighBlock + 0x4000;
    static const word cGraphicsAddrA = 0x8000;
    static const word cGraphicsAddrB = 0xF000;
};


#endif // MEMMAPPER_H