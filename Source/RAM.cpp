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
#include "RAM.h"
#include <wx/file.h>
#include <cstring>


/*! \p config_ must specify the size attribute on the <device>, and can
 *     specify a filename attribute that names the file that should be loaded as
 *     the initial data.
 */
RAM::RAM(Microbee &, const TiXmlElement &config_) :
    config(config_)
{
    int s;

    if (config.Attribute("size", &s) == NULL)
        throw ConfigError(&config, "RAM missing size attribute");
    if (s <= 0)
        throw ConfigError(&config, "RAM size attribute must be positive");

    size = s;

    memory = new byte[size];
}


RAM::RAM(unsigned int size_) : 
    MemoryDevice(size_),
    config("device")
{
    memory = new byte[size];
}



RAM::~RAM()
{
    delete[] memory;
}


void RAM::Reset()
{
    memset(memory, 0, size);

    const char *file;
    if ((file = config.Attribute("filename")) != NULL)
    {
        try
        {
            LoadRAM(file);
        }
        catch (FileNotFound &)
        {
            throw ConfigError(&config, "Unable to open RAM file");
        }
    }
}


 /*! \throws FileNotFound if the file cannot be opened for reading
 */
void RAM::LoadRAM(const char *filename)
{
    if (*filename == '\0')  // If no filename is passed, ROM should still be created but is to be empty
        return;

    wxFile ram_file(filename);

    if (!ram_file.IsOpened())
        throw FileNotFound(filename);

    wxFileOffset len = ram_file.Length();
    if (size < len)
        ram_file.Read(memory, size);
    else
        ram_file.Read(memory, len);
}

void RAM::SaveState(BinaryWriter& writer)
{
    writer.WriteBuffer(this->memory, this->size);
}

void RAM::RestoreState(BinaryReader& reader)
{
    reader.ReadBuffer(this->memory, this->size);
}

