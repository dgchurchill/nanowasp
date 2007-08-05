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

#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

#include "Device.h"


/*! \brief Base class for all devices which appear in the memory address space
 *
 *  \note The emulated device may not actually be a memory in a real sense -
 *        this class is for devices which appear in the memory address space
 *        regardless of their actual operation.
 */
class MemoryDevice : public Device
{
public:
    //! Constructs a MemoryDevice that occupies \p size_ contiguous bytes of the address space
    MemoryDevice(unsigned int size_ = 0) : size(size_) {};

    //! Returns the amount of address space occupied by this device in bytes
    unsigned int GetSize() { return size; }

    /*! \brief Writes the byte \p val into the memory device at \p addr
     *
     *  \note \p addr is relative to the start of the device in the address space.
     */
    virtual void Write(word addr, byte val) = 0;
    /*! \brief Reads the byte at \p addr from the memory device
     *
     *  \note \p addr is relative to the start of the device in the address space.
     */
    virtual byte Read(word addr) = 0;


protected:
    unsigned int size;  //!< Number of bytes that the devices occupies in the address space
};


#endif // MEMORYDEVICE_H