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

#ifndef DISK_H
#define DISK_H

#include <libdsk.h>


/*! \brief Represents a magnetic disk
 *
 *  The underlying implementation is provided by the libdsk library, so there is the potential
 *  to allow reading directly from real disks or directories of files masquerading as disks, as
 *  well as plain old disk images.
 */
class Disk
{
public:
    //! Constructs a Disk object using file \p name for the data
    Disk(const char *name);
    //! Creates a new DSK file called \p name and uses it to construct a Disk object
    Disk(const char *name, int heads, int cyls, int sects, int sect_size);
    ~Disk();

    //! Reads an ID field from the disk
    bool ReadIDField(unsigned char *buf, byte head, byte cyl);
    //! Read a sector from the disk
    bool ReadSector(unsigned char *buf, byte head, byte cyl, byte sect);
    //! Write a sector to the disk
    bool WriteSector(unsigned char *buf, byte head, byte cyl, byte sect);
    //! Format a track on the disk
    bool FormatTrack(DSK_FORMAT *format, byte filler, unsigned int num_sectors, byte head, byte cyl);

    //! Returns the write-protect status of the disk
    bool IsProtected();

    unsigned int SectorsPerTrack() const { return geom.dg_sectors; }

    //! Converts a sector size to a type code
    static byte SectorSizeToType(size_t size);
    //! Converts a sector type code to a size
    static unsigned int SectorTypeToSize(byte type);


private:
	DSK_PDRIVER disk;  //!< libdsk disk handle
	DSK_GEOMETRY geom;  //!< libdsk disk geometry

    // Private copy constuctor and assigment operator to prevent copies
    Disk(const Disk &);
    Disk& operator= (const Disk &);
};

#endif // DISK_H