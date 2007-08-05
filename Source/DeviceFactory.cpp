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
#include "DeviceFactory.h"

#include "Z80/Z80CPU.h"
#include "ROM.h"
#include "RAM.h"
#include "MemMapper.h"
#include "CRTCMemory.h"
#include "CRTC.h"
#include "LatchROM.h"
#include "Keyboard.h"
#include "FDC.h"
#include "Drives.h"


DeviceFactory::DeviceFactory()
{
    funcmap["Z80CPU"] = createZ80CPU;
    funcmap["Keyboard"] = createKeyboard;

    funcmap["CRTCMemory"] = createCRTCMemory;
    funcmap["RAM"] = createRAM;
    funcmap["ROM"] = createROM;

    funcmap["CRTC"] = createCRTC;
    funcmap["Drives"] = createDrives;
    funcmap["FDC"] = createFDC;
    funcmap["LatchROM"] = createLatchROM;
    funcmap["MemMapper"] = createMemMapper;
}


/*! \throws BadDeviceClass if \p id is an unknown Device class
 */
Device *DeviceFactory::createDevice(const char *id, Microbee &mbee, const TiXmlElement &config)
{
    factoryFunc ff = funcmap[id];

    if (ff != NULL)
        return ff(mbee, config);
    else
        throw BadDeviceClass(id);
}


Device *DeviceFactory::createZ80CPU(Microbee &mbee, const TiXmlElement &config)
{ 
    return new Z80CPU(mbee, config); 
}

Device *DeviceFactory::createKeyboard(Microbee &mbee, const TiXmlElement &config)
{ 
    return new Keyboard(mbee, config); 
}


Device *DeviceFactory::createCRTCMemory(Microbee &mbee, const TiXmlElement &config)
{ 
    return new CRTCMemory(mbee, config); 
}

Device *DeviceFactory::createRAM(Microbee &mbee, const TiXmlElement &config)
{ 
    return new RAM(mbee, config); 
}

Device *DeviceFactory::createROM(Microbee &mbee, const TiXmlElement &config)
{ 
    return new ROM(mbee, config); 
}


Device *DeviceFactory::createCRTC(Microbee &mbee, const TiXmlElement &config)
{ 
    return new CRTC(mbee, config); 
}

Device *DeviceFactory::createDrives(Microbee &mbee, const TiXmlElement &config)
{ 
    return new Drives(mbee, config); 
}

Device *DeviceFactory::createFDC(Microbee &mbee, const TiXmlElement &config)
{ 
    return new FDC(mbee, config); 
}

Device *DeviceFactory::createLatchROM(Microbee &mbee, const TiXmlElement &config)
{ 
    return new LatchROM(mbee, config); 
}

Device *DeviceFactory::createMemMapper(Microbee &mbee, const TiXmlElement &config)
{ 
    return new MemMapper(mbee, config); 
}
