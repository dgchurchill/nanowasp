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

#ifndef DEVICE_H
#define DEVICE_H

#include "tinyxml/tinyxml.h"
#include "Microbee.h"


/*! \brief Base class for all emulated devices
 *
 *  \note Generally, emulated devices will derive from a specialisation of this class
 *        rather than directly from this class (e.g. MemoryDevice, PortDevice).
 */
class Device
{
public:
    virtual ~Device() {};

    /*! \brief Performs any initialisation that requires access to other system devices
     *
     *  This function will be called on each device after all devices in the system have
     *  been constructed, but before any other action takes place (e.g. prior to any Reset() ).
     *  Generally this is used to initialise any references to other
     *  devices that are required by the current device.  The constructor should be used
     *  for any initialisation that does not depend on other devices.
     *
     *  \throws ConfigError if a configuration error is detected
     *  \throws UnknownDevice if a <connect> element specifies a destination device that doesn't 
     *          exist.  This saves each LateInit() from having to catch and rethrow these
     *          exceptions as ConfigErrors, which is what they really are (they will be rethrown
     *          as ConfigErrors by Microbee).
     *  \throws BadDeviceClass if a <connect> element specifies a destination device of the wrong
     *          type.  Discussion for UnknownDevice also applies here.
     */
    virtual void LateInit() {}


    /*! \brief Resets the device
     *
     *  A reset request will only be made after all Devices in the emulated system are
     *  constructed and LateInit()ed.
     *
     *  \warning The implementation of this function should not rely on the state of other
     *           Devices in the system. In particular, the order that indiviual Devices are
     *           reset during a reset of the entire emulated system is not specified.
     */
    virtual void Reset() {}


    /*! \brief Runs the device for the given number of microseconds, starting at the specified emulated time
     *
     *  \returns The maximum number of simulated microseconds that may elapse before the device should be executed
     *           again.  The device is guaranteed to be executed again within this period.  If the device returns
     *           0 then it won't be considered when determining the length of the next run cycle.  If all devices
     *           return 0 then the next run cycle will be of default length.
     *
     *  This function will be called regularly by the main loop of the emulator for
     *  each Device that is on its run list.  \p micros indicates how many simulated microseconds the
     *  device should execute for.  If the device does not need to be called regularly then it
     *  doesn't need to reimplement this function (and should not be placed on the run list).
     *
     *  \sa Executable()
     */
    virtual Microbee::time_t Execute(Microbee::time_t time, Microbee::time_t micros) { UNREFERENCED_PARAMETER(time); UNREFERENCED_PARAMETER(micros); return 0; }


    /*! \brief Returns true if the device needs to be on the run list
     *
     *  The default implementation returns false, so this only needs to be overriden if
     *  Execute() is overriden.
     */
    virtual bool Executable() { return false; }


    /*! \brief Returns the emulated time in microseconds that the Device has run been Execute()d to
     *
     *  If this function is called while the Device is currently Execute()ing, it must also include any
     *  time that has been emulated so far in the current call.  Devices which haven't reimplemented
     *  Execute() don't need to reimplement this function either.
     */
    virtual Microbee::time_t GetTime() { return 0; };
};


#endif // DEVICE_H