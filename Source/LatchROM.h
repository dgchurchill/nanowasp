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

#ifndef LATCHROM_H
#define LATCHROM_H

#include "PortDevice.h"


/*! \brief Emulates the 'latch ROM' flip-flop
 *
 *  This flip-flop is used to select between the video RAM and character ROM
 *  in the system address space.  When clear, video RAM is selected.  When set,
 *  the character ROM is selected.  It also disables normal scanning of the keyboard
 *  when set.
 */
class LatchROM : public PortDevice
{
public:
    LatchROM(Microbee &, const TiXmlElement &) : PortDevice(1) { };

    virtual void Reset();

    virtual void PortWrite(word addr, byte val);
    virtual byte PortRead(word addr);

    virtual void SaveState(BinaryWriter&);
    virtual void RestoreState(BinaryReader&);

    //! Returns the current setting of the flip-flop
    bool GetLatch() const { return latch; }


private:
    bool latch;  //!< Current state of the flip-flop
};


#endif // LATCHROM_H