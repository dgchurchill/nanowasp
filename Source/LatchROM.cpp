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


#include "stdafx.h"
#include "LatchROM.h"


void LatchROM::Reset()
{
    latch = false;
}


void LatchROM::PortWrite(word addr, byte val)
{
    UNREFERENCED_PARAMETER(addr);

    latch = getBit(val, 0) == 1;
}


byte LatchROM::PortRead(word addr)
{
    UNREFERENCED_PARAMETER(addr);

    return 0;  // LatchROM flip-flip can't be read
}

void LatchROM::SaveState(BinaryWriter &writer)
{
    writer.WriteBool(this->latch);
}

void LatchROM::RestoreState(BinaryReader& reader)
{
    this->latch = reader.ReadBool();
}