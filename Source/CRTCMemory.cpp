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
#include "CRTCMemory.h"
#include "Microbee.h"
#include "LatchROM.h"
#include "CRTC.h"
#include "RAM.h"
#include "ROM.h"


/*! Initialises video RAM, PCG RAM.  Loads character ROM data and arranges it
    in memory suitable for OpenGL. \p config_ must contain a <connect> to the associated 
    CRTC and LatchROM devices, and a charrom attribute specifying the file containing the
    char ROM data. */
CRTCMemory::CRTCMemory(Microbee &mbee_, const TiXmlElement &config_) : 
    MemoryDevice(cGraphicsMemSize),
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    latch_rom(NULL),
    crt_c(NULL),
    video_ram(cVideoRAMSize),
    pcg_ram(cPCGRAMSize),
    char_rom(mbee_, cCharROMSize)
{
    // Load the char ROM data
    const char *file;
    if ((file = config_.Attribute("charrom")) == NULL)
        throw ConfigError(&config_, "CRTCMemory missing charrom attribute");

    try
    {
        char_rom.LoadROM(file);
    }
    catch (FileNotFound &)
    {
        throw ConfigError(&config_, std::string("Unable to open Character ROM file \"") + file + "\"");
    }

    // Reorder char rom bytes so they represent OpenGL bitmaps
    for (int i = 0; i < cCharROMSize / cBitmapSize; ++i)
        for (int j = 0; j < cBitmapSize / 2; ++j)
        {
            byte tmp = char_rom.memory[i * cBitmapSize+j];
            char_rom.memory[i * cBitmapSize + j] = char_rom.memory[XlatAddress(i * cBitmapSize + j)];
            char_rom.memory[XlatAddress(i * cBitmapSize+j)] = tmp;
        }
}


void CRTCMemory::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "CRTCMemory <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "CRTC")
            crt_c = mbee.GetDevice<CRTC>(dest);
        else if (type_str == "LatchROM")
            latch_rom = mbee.GetDevice<LatchROM>(dest);
    }

    if (crt_c == NULL)
        throw ConfigError(&xml_config, "CRTCMemory missing CRTC connection");
    if (latch_rom == NULL)
        throw ConfigError(&xml_config, "CRTCMemory missing LatchROM connection");
}


/*! Writes a byte into graphics memory.  Bytes written to the PCG RAM are
    reordered so that OpenGL can use the bitmaps directly.  The character
    ROM cannot be written to. */
void CRTCMemory::Write(word addr, byte val)
{
    if (addr >= cVideoRAMSize)
        pcg_ram.Write(XlatAddress(addr % cPCGRAMSize), val);
    else if (latch_rom->GetLatch())
        ;  // no-op, writing to char rom
    else
        video_ram.Write(addr % cVideoRAMSize, val);
}


/*! Reads a byte from graphics memory.  Bytes read from the char ROM or PCG
    RAM are reordered again so the underlying storage scheme is not visible
    to the Microbee software (i.e. everything just looks normal from its
    perspective).  

    \note In the real system MA13 from the CRT controller is used to select between the low/high 2k of the
    character ROM.  The MA lines are used to scan through the video memory when rendering
    the screen.  Because the MA line signals aren't emulated completely the disp_start value is used
    here instead, which is what programs would set to change the char ROM bank normally. */
byte CRTCMemory::Read(word addr)
{
    if (addr >= cVideoRAMSize)
        return pcg_ram.Read(XlatAddress(addr % cPCGRAMSize));
    else if (latch_rom->GetLatch())
        return char_rom.Read(XlatAddress(getBit(crt_c->GetDispStart(), cBitMA13) * cVideoRAMSize + addr % cVideoRAMSize));
    else
        return video_ram.Read(addr);
}


/*! The bitmap referenced by the return value is in OpenGL format.

    \param addr The memory address from the CRTC (includes signal that selects char ROM bank)
    \param scans_per_row Current scans_per_row (required because of the munging to OpenGL format)
 */
const unsigned char *CRTCMemory::GetCharBitmap(word addr, word scans_per_row)
{
    byte b = video_ram.Read(addr % cVideoRAMSize);

    if (getBit(b, cBitPCG))
        return &pcg_ram.memory[(getBits(b, cIndexOfs, cIndexSize) + 1) * cBitmapSize - scans_per_row];
    else
        return &char_rom.memory[getBit(addr, cBitMA13) * cVideoRAMSize + 
                                ((getBits(b, cIndexOfs, cIndexSize) + 1) * cBitmapSize - scans_per_row)];
}

void CRTCMemory::SaveState(BinaryWriter& writer)
{
    this->video_ram.SaveState(writer);
    this->pcg_ram.SaveState(writer);
}

void CRTCMemory::RestoreState(BinaryReader& reader)
{
    this->video_ram.RestoreState(reader);
    this->pcg_ram.RestoreState(reader);
}

word CRTCMemory::XlatAddress(word addr)
{
    word chr = addr / cBitmapSize;
    word ofs = addr % cBitmapSize;
    return (chr + 1) * cBitmapSize - ofs - 1;
}
