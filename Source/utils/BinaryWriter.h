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

#ifndef BINARYWRITER_H
#define BINARYWRITER_H

#include <iostream>


/*! \brief Reads binary data to a stream
 *
 *  Data is stored in little-endian format.
 */
class BinaryWriter
{
public:
    // TODO: Ensure that the stream is a stream of unsigned char?
    explicit BinaryWriter(std::ostream& stream);
    
    // TODO: Check that stream is not in a error state at the end of each of these methods.
    void WriteByte(unsigned char b);
    void WriteWord(unsigned short w);
    void WriteBool(bool b);
    void WriteBuffer(const unsigned char* buffer, int length);
    
private:
    std::ostream& stream;
};

#endif // BINARYWRITER_H
