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

#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include "tinyxml/tinyxml.h"
#include <map>

class Device;
class Microbee;


/*! \brief Allows creation of a Device subclass by name
 *
 *  Each instantiable Device must have a corresponding createXxx function defined
 *  here, and an entry added to the funcmap in the DeviceFactory() constructor.
 *  The createXxx function name and the entry in the funcmap must match the Device's
 *  class name for consistency.  The devices NullMemory and NullPort are not included
 *  here as there should be no reason for them to be instantiated in a context where
 *  the device type is unknown.
 */
class DeviceFactory
{
public:
    DeviceFactory();

    //! Creates a Device of type \p id
    Device *createDevice(const char *id, Microbee &mbee, const TiXmlElement &config);


private:
    typedef Device *(*factoryFunc)(Microbee &mbee, const TiXmlElement &config);
    std::map<std::string, factoryFunc> funcmap;

    static Device *createZ80CPU(Microbee &mbee, const TiXmlElement &config);
    static Device *createKeyboard(Microbee &mbee, const TiXmlElement &config);

    static Device *createCRTCMemory(Microbee &mbee, const TiXmlElement &config);
    static Device *createRAM(Microbee &mbee, const TiXmlElement &config);
    static Device *createROM(Microbee &mbee, const TiXmlElement &config);

    static Device *createCRTC(Microbee &mbee, const TiXmlElement &config);
    static Device *createDrives(Microbee &mbee, const TiXmlElement &config);
    static Device *createFDC(Microbee &mbee, const TiXmlElement &config);
    static Device *createLatchROM(Microbee &mbee, const TiXmlElement &config);
    static Device *createMemMapper(Microbee &mbee, const TiXmlElement &config);
};

#endif // DEVICEFACTORY_H