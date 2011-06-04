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

#ifndef RAM_H
#define RAM_H

#include <vector>
#include "MemoryDevice.h"


/*! \brief Emulates a %RAM chip */
class RAM : public MemoryDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    RAM(Microbee &, const TiXmlElement &config_);

    //! Creates a new RAM device of \p size_ bytes, initialised to 0
    RAM(unsigned int size_);


    virtual void Reset();

    virtual void Write(word addr, byte val) { memory[addr] = val; };
    virtual byte Read(word addr) { return memory[addr]; };

    std::vector<byte> memory;  //!< The contents of the RAM, plain array for speed

    virtual void SaveState(BinaryWriter&);
    virtual void RestoreState(BinaryReader&);

private:
    TiXmlElement config;
    void LoadRAM(const char *filename);
};


#endif // RAM_H