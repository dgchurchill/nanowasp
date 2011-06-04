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

#ifndef CRTCMEMORY_H
#define CRTCMEMORY_H

#include "MemoryDevice.h"
#include "RAM.h"
#include "ROM.h"
#include <vector>

class Microbee;
class LatchROM;
class CRTC;


/*! \brief Emulates the memory used by the graphics system
 *
 *  The graphics memory consists of three components:
 *    - Video RAM, 2kB (i.e. which character is to be shown at each screen location)
 *    - Character ROM, 4kB (bitmap data for built-in characters)
 *    - PCG RAM, 2kB (Programmable Character Graphics RAM: bitmap data for user characters)
 *
 *  The three memories are implemented as a single MemoryDevice because they effectively
 *  form a single system compenent.  This allows functions such as GetCharBitmap() to be
 *  defined sensibly within the class.  The graphics memory is also treated as a single
 *  unit by the Microbee's memory mapper.
 *
 *  Bytes in the video RAM reference character bitmaps defined in the character ROM and
 *  PCG RAM.  The high bit of a video RAM byte selects between the char ROM and PCG RAM
 *  for that character.  The low seven bits index the character data within that memory.
 *  Each character bitmap is stored as 16 bytes, with the bits of each byte defining
 *  an eight pixel row of the character.
 *
 *  The PCG RAM is 2kB in size, allowing 2048 / 16  = 128 character bitmaps to be stored.
 *  In constrast the character ROM is 4kB, providing storage for 256 character bitmaps.
 *  As only 7 bits of the video RAM are used to index a character, the graphics memory
 *  system uses an address line from the CRTC to select between the high and low banks.
 *
 *
 *  The graphics memory appears in the system address space as two blocks of 2kB.  The 
 *  lower 2kB contains either the video RAM or the character ROM (the LatchROM device is
 *  used to select between them), while the upper 2kB contains the PCG RAM.
 */
class CRTCMemory : public MemoryDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    CRTCMemory(Microbee &mbee_, const TiXmlElement &config_);

    virtual void LateInit();


    virtual void Write(word addr, byte val);
    virtual byte Read(word addr);

    //! Returns a pointer to the character bitmap referenced by the <em>video RAM</em> byte at \p addr
    const unsigned char *GetCharBitmap(word addr, word scans_per_row);

    virtual void SaveState(BinaryWriter&);
    virtual void RestoreState(BinaryReader&);

    static const word cBitmapSize = 16;  //!< Character bitmap length in bytes


private:
    Microbee &mbee;  //!< Owning Microbee
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config
    LatchROM *latch_rom;  //!< Connection to the LatchROM device, selects between char ROM and PCG RAM in system memory
    CRTC *crt_c;  //!< Connection to the CRTC device, CRTC signal selects between low and high char ROM banks

    RAM video_ram; //!< The video RAM
    RAM pcg_ram;   //!< The PCG RAM
    ROM char_rom;  //!< The character ROM

    //! Converts addresses so that bitmap data is stored appropriate for OpenGL
    static word XlatAddress(word addr);

    static void ReorderBitmaps(std::vector<byte>& memory);
    
    static const word cGraphicsMemSize = 4096;
    static const word cVideoRAMSize = 2048;
    static const word cPCGRAMSize = 2048;
    static const word cCharROMSize = 4096;
    static const word cBitMA13 = 13;
    static const word cBitPCG = 7;
    static const word cIndexOfs = 0;
    static const word cIndexSize = 7;
};


#endif // CRTCMEMORY_H