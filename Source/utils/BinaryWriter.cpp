/*  Nanowasp - A Microbee emulator
 *  Copyright (C) 2000-2011 David G. Churchill
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

#include "BinaryWriter.h"

BinaryWriter::BinaryWriter(std::ostream& stream) :
    stream(stream)
{
}

void BinaryWriter::WriteByte(unsigned char b)
{
    this->stream.put(b);
}

void BinaryWriter::WriteWord(unsigned short w)
{
    this->WriteByte(w & 0xFF);
    this->WriteByte(w >> 8);
}

void BinaryWriter::WriteBool(bool b)
{
    this->WriteByte(b ? 1 : 0);
}

void BinaryWriter::WriteBuffer(const unsigned char* buffer, int length)
{
    this->stream.write((const char*)buffer, length);
}
