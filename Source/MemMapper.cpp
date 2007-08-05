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
#include "MemMapper.h"

#include "Microbee.h"
#include "Z80/Z80CPU.h"
#include "RAM.h"
#include "ROM.h"
#include "CRTCMemory.h"


/*! \p config_ must contain a <connect> for the associated Z80 device, and each memory bank. */
MemMapper::MemMapper(Microbee &mbee_, const TiXmlElement &config_) :
    PortDevice(1),
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    z80(NULL),
    bank0(NULL),
    bank1(NULL),
    bank2(NULL),
    bank3(NULL),
    rom1(NULL),
    rom2(NULL),
    rom3(NULL),
    crtcmem(NULL)
{
}


void MemMapper::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "MemMapper <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "Z80CPU")
            z80 = mbee.GetDevice<Z80CPU>(dest);
        else if (type_str == "Bank0")
            bank0 = mbee.GetDevice<RAM>(dest);
        else if (type_str == "Bank1")
            bank1 = mbee.GetDevice<RAM>(dest);
        else if (type_str == "Bank2")
            bank2 = mbee.GetDevice<RAM>(dest);
        else if (type_str == "Bank3")
            bank3 = mbee.GetDevice<RAM>(dest);
        else if (type_str == "ROM1")
            rom1 = mbee.GetDevice<ROM>(dest);
        else if (type_str == "ROM2")
            rom2 = mbee.GetDevice<ROM>(dest);
        else if (type_str == "ROM3")
            rom3 = mbee.GetDevice<ROM>(dest);
        else if (type_str == "CRTCMemory")
            crtcmem = mbee.GetDevice<CRTCMemory>(dest);
    }

    if (z80 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing Z80CPU connection");
    if (bank0 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing Bank0 connection");
    if (bank1 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing Bank1 connection");
    if (bank2 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing Bank2 connection");
    if (bank3 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing Bank3 connection");
    if (rom1 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing ROM1 connection");
    if (rom2 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing ROM2 connection");
    if (rom3 == NULL)
        throw ConfigError(&xml_config, "MemMapper missing ROM3 connection");
    if (crtcmem == NULL)
        throw ConfigError(&xml_config, "MemMapper missing CRTCMemory connection");
}


void MemMapper::Reset()
{
    PortWrite(0, 0);
}


void MemMapper::PortWrite(word addr, byte val)
{
    UNREFERENCED_PARAMETER(addr);

    // Lower 32k
    switch (val & cBank)
    {
    case 0:
    case 6:
        z80->RegMemoryDevice(cLowBlock, bank1); 
        break;

    case 1:
    case 7:
        z80->RegMemoryDevice(cLowBlock, bank3); 
        break;

    case 2:
    case 4:
        z80->RegMemoryDevice(cLowBlock, bank0); 
        break;

    case 3:
    case 5:
        z80->RegMemoryDevice(cLowBlock, bank2); 
        break;
    }


    // Upper 32k
    if (val & cROMDisable)
        z80->RegMemoryDevice(cHighBlock, bank1);
    else
    {
        z80->RegMemoryDevice(cHighBlockA, rom1);

        if (val & cROMSelect)
            z80->RegMemoryDevice(cHighBlockB, rom3);
        else
            z80->RegMemoryDevice(cHighBlockB, rom2);
    }


    // Video RAM - *this must be last* so that it overrides any other handlers already registered
    if (!(val & cVideoRAMDisable))
    {
        // Enable video RAM
        if (val & cVideoRAMLoc)
            z80->RegMemoryDevice(cGraphicsAddrA, crtcmem);
        else
            z80->RegMemoryDevice(cGraphicsAddrB, crtcmem);
    }
}


byte MemMapper::PortRead(word addr)
{
    UNREFERENCED_PARAMETER(addr);

    return 0;  // The memory mapper setting is not readable
}
