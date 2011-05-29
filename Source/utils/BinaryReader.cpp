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

#include "BinaryReader.h"

BinaryReader::BinaryReader(std::istream& stream) :
    stream(stream)
{
}

unsigned char BinaryReader::ReadByte()
{
    return this->stream.get();
}

unsigned short BinaryReader::ReadWord()
{
    unsigned short result = 0;
    result |= this->stream.get();
    result |= this->stream.get() << 8;
    return result;
}

bool BinaryReader::ReadBool()
{
    return this->ReadByte() != 0;
}

void BinaryReader::ReadBuffer(unsigned char* buffer, int length)
{
    this->stream.read((char*)buffer, length);
}
