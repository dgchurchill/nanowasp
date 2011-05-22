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
#include "ROM.h"
#include <wx/file.h>
#include "Exceptions.h"


/*! \p config_ must specify size and filename attributes on the <device> */
ROM::ROM(Microbee &mbee_, const TiXmlElement &config_) :
    mbee(mbee_)
{
    int s;

    if (config_.Attribute("size", &s) == NULL)
        throw ConfigError(&config_, "ROM missing size attribute");
    if (s <= 0)
        throw ConfigError(&config_, "ROM size attribute must be positive");
    size = s;

    const char *file;
    if ((file = config_.Attribute("filename")) == NULL)
        throw ConfigError(&config_, "ROM missing filename attribute");

    memory.resize(size, 0);

    try
    {
        LoadROM(file);
    }
    catch (FileNotFound &)
    {
        throw ConfigError(&config_, "Unable to open ROM file");
    }
}


ROM::ROM(Microbee &mbee_, unsigned int size_) :
    MemoryDevice(size_),
    memory(size_, 0),
    mbee(mbee_)
{
}


/*! If the file specified is smaller than the size of the ROM then the file data will be
    aliased multiple times within the ROM's address space.  A read at address
    x will return the byte in the image at x % filesize.  If \p filename is NULL
    or points to an empty string then the ROM contents will be cleared to zero.

    \throws FileNotFound if the file cannot be opened for reading
 */
void ROM::LoadROM(const char *filename)
{
    if (filename == NULL || *filename == '\0')  // If no filename is passed then clear out the ROM
    {
        memory.clear();
        memory.resize(size, 0);
        return;
    }
    
    wxFile rom_file(mbee.GetConfigFileName().GetPath(wxPATH_GET_SEPARATOR) + filename);

    if (!rom_file.IsOpened())
        throw FileNotFound(filename);

    wxFileOffset len = rom_file.Length();
    if (size < len)
    {
        rom_file.Read(&memory[0], size);
    }
    else
    {
        rom_file.Read(&memory[0], len);
        for (unsigned int i = len; i < size; i++)  // Alias ROM to memory addresses in the ROM space
            memory[i] = memory[i % len];
    }
}
