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
#include "Disk.h"

#include <algorithm>
#include <iostream>
#include <vector>


/*! \throws DiskImageError if disk image \p name could not be opened */
Disk::Disk(const char *name) :
    disk(NULL)
{
    char *type = "dsk";

    std::string name_str(name);
    if (name_str.length() >= 4)
    {
        transform(name_str.end() - 4, name_str.end(), name_str.end() - 4, tolower);
        if (name_str.compare(name_str.length() - 4, 4, ".img") == 0)
            type = "nanowasp";
    }

    if (dsk_open(&disk, name, type, NULL) != DSK_ERR_OK)
        throw DiskImageError();

    memset(&geom, 0, sizeof(DSK_GEOMETRY));
    // TODO: These values shouldn't be specified explicitly here
    geom.dg_cylinders = 40;
    geom.dg_heads = 2;
    geom.dg_sectors = 10;
    geom.dg_secbase = 1;
    geom.dg_secsize = 512;
//    dg_stdformat(&geom, FMT_MBEE400, NULL, NULL);
}


/*! \throws DiskImageError if disk image \p name could not be created */
Disk::Disk(const char *name, int heads, int cyls, int sects, int sect_size)
{
    std::vector<DSK_FORMAT> fmt(sects);

    if (dsk_creat(&disk, name, "dsk", NULL) != DSK_ERR_OK)
        throw DiskImageError();

    memset(&geom, 0, sizeof(DSK_GEOMETRY));
    geom.dg_cylinders = cyls;
    geom.dg_heads = heads;
    geom.dg_sectors = sects;
    geom.dg_secbase = 1;
    geom.dg_secsize = sect_size;

    // Formatting is done here just so that the .dsk file is big enough (the libdsk driver
    // for .dsk files can't handle a track growing in size).  The disk will have to be formatted
    // properly by the Microbee software.
    for (int c = 0; c < cyls; ++c)
        for (int h = 0; h < heads; ++h)
        {
            for (int s = 0; s < sects; s++)
            {
                fmt[s].fmt_cylinder = c;
                fmt[s].fmt_head = h;
                fmt[s].fmt_sector = geom.dg_secbase + s;
                fmt[s].fmt_secsize = sect_size;
            }

            dsk_pformat(disk, &geom, c, h, &fmt[0], 0xE5 /* default cp/m fill */);  
        }
}


Disk::~Disk()
{
   dsk_close(&disk);
}


/*! \param buf    The buffer to store the data read (must be at least as big as the disk's sector size)
    \param head   The physical head to read from
    \param cyl    The physical cylinder to read from
    \param sect   The logical sector mark to look for

    \returns True if read was successful
 */
bool Disk::ReadSector(unsigned char *buf, byte head, byte cyl, byte sect)
{
    return dsk_pread(disk, &geom, buf, cyl, head, sect) == DSK_ERR_OK;
}


/*! \param buf    The buffer containing the data to write (must be at least as big as the disk's sector size)
    \param head   The physical head to write to
    \param cyl    The physical cylinder to write to
    \param sect   The logical sector mark to look for

    \returns True if write was successful
 */
bool Disk::WriteSector(unsigned char *buf, byte head, byte cyl, byte sect)
{
    return dsk_pwrite(disk, &geom, buf, cyl, head, sect) == DSK_ERR_OK;
}


/*! \param buf    The buffer to store the data read (must be at least 6 bytes big)
    \param head   The physical head to read from
    \param cyl    The physical cylinder to read from

    \returns True if read was successful
 */
bool Disk::ReadIDField(unsigned char *buf, byte head, byte cyl)
{
    DSK_FORMAT dsk_fmt;

    if (dsk_psecid(disk, &geom, cyl, head, &dsk_fmt) != DSK_ERR_OK)
        return false;

    buf[0] = dsk_fmt.fmt_cylinder;
    buf[1] = dsk_fmt.fmt_head;
    buf[2] = dsk_fmt.fmt_sector;
    buf[3] = SectorSizeToType(dsk_fmt.fmt_secsize);
    buf[4] = buf[5] = 0xFF;  // TODO: Implement CRC

    return true;
}

bool Disk::FormatTrack(DSK_FORMAT *format, byte filler, unsigned int num_sectors, byte head, byte cyl)
{
    geom.dg_sectors = num_sectors;  // This is probably an abuse, but so long as none of the logical libdsk commands are used it should be ok
    return dsk_pformat(disk, &geom, cyl, head, format, filler) == DSK_ERR_OK;
}


bool Disk::IsProtected()
{
    unsigned char status;

    if (dsk_drive_status(disk, &geom, 0, &status) == DSK_ERR_OK)
        return (status & DSK_ST3_RO) != 0;
    else
        return true;  // Write protected by default
}


byte Disk::SectorSizeToType(size_t size)
{
    switch (size)
    {
    case 128:
        return 0;
        break;
    case 256:
        return 1;
        break;
    case 512:
        return 2;
        break;
    case 1024:
        return 3;
        break;
    default:
        return 0xFF;
    }
}


unsigned int Disk::SectorTypeToSize(byte type)
{
    switch (type)
    {
    case 0:
        return 128;
        break;
    case 1:
        return 256;
        break;
    case 2:
        return 512;
        break;
    case 3:
        return 1024;
        break;
    default:
        return 0;
    }
}
