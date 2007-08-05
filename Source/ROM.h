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

#ifndef ROM_H
#define ROM_H

#include <vector>
#include "MemoryDevice.h"


/*! \brief Emulates a %ROM chip */
class ROM : public MemoryDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    ROM(Microbee &, const TiXmlElement &config_);

    /*! \brief Creates a new empty ROM device of \p size_ bytes.  Contents should be loaded
     *         using LoadROM() */
    ROM(unsigned int size_);

    //! Loads the ROM with the contents of \p filename
    void LoadROM(const char *filename);


    virtual void Write(word addr, byte val) { UNREFERENCED_PARAMETER(addr); UNREFERENCED_PARAMETER(val); } // No action when writing to ROM
    virtual byte Read(word addr) { return memory[addr]; };

    std::vector<byte> memory;  //!< The contents of the ROM
};


#endif // ROM_H