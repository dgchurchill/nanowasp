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

#ifndef PORTDEVICE_H
#define PORTDEVICE_H

#include "Device.h"


//! Base class for all devices which appear in the port address space
class PortDevice : public Device
{
public:
    //! Constructs a PortDevice that occupies \p size_ contiguous ports in the port address space
    PortDevice(unsigned int size_ = 0) : size(size_) {};

    //! Returns the number of ports by this device
    unsigned int GetSize() { return size; }

    /*! \brief Writes the byte \p val into port \p addr of the device
     *
     *  \note \p addr is relative to the start of the device in the port address space.
     */
    virtual void PortWrite(word addr, byte val) = 0;
    /*! \brief Reads the byte from port \p addr of the device
     *
     *  \note \p addr is relative to the start of the device in the port address space.
     */
    virtual byte PortRead(word addr) = 0;


protected:
    unsigned int size;  //!< Number of bytes that the devices occupies in the address space
};


#endif // PORTDEVICE_H