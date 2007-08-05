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

#ifndef NULLPORT_H
#define NULLPORT_H

#include "PortDevice.h"


/*! \brief PortDevice for unmapped addresses, ignores all write and returns 0 for reads */
class NullPort : public PortDevice
{
public:
    NullPort(unsigned int s) : PortDevice(s) {};

    virtual void PortWrite(word addr, byte val) { UNREFERENCED_PARAMETER(addr);  UNREFERENCED_PARAMETER(val); };
    virtual byte PortRead(word addr) { UNREFERENCED_PARAMETER(addr);  return 0; };
};


#endif // NULLPORT_H