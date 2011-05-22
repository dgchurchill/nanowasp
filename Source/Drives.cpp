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
#include "Drives.h"

#include "Microbee.h"
#include "Disk.h"
#include "FDC.h"


/*! \p config_ must contain a <connect> to the associated FDC device. 
 *  It may also contain <disk> elements specifying disks to be loaded.
 */
Drives::Drives(Microbee &mbee_, const TiXmlElement &config_) :
    PortDevice(cNumPorts),
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    fdc(NULL),
    disks(cNumDrives),
    cyl(cNumDrives)
{
}


Drives::~Drives()
{
    std::vector<Disk*>::iterator it = disks.begin();

    for (; it != disks.end(); it++)
        delete *it;
}


void Drives::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "Drives <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "FDC")
            fdc = mbee.GetDevice<FDC>(dest);
    }

    if (fdc == NULL)
        throw ConfigError(&xml_config, "Drives missing FDC connection");


    // Load any Disks specified
    for (TiXmlElement *el = xml_config.FirstChildElement("disk"); el != NULL; el = el->NextSiblingElement("disk"))
    {
        int drv;
        if (el->Attribute("drive", &drv) == NULL)
            throw ConfigError(el, "<disk> missing drive attribute");

        const char *file = el->Attribute("filename");
        if (file == NULL)
            throw ConfigError(el, "<disk> missing filename attribute");

        try
        {
            LoadDisk(drv, mbee.GetConfigFileName().GetPath(wxPATH_GET_SEPARATOR) + file);
        }
        catch (OutOfRange &)
        {
            throw ConfigError(el, "<disk> specifies an invalid drive");
        }
        catch (DiskImageError &)
        {
            throw ConfigError(el, "Disk image could not be loaded");
        }
    }
}


void Drives::Reset()
{
    ctrl_side = 0;
    ctrl_drive = 0;
    ctrl_ddense = false;
    SeekTrackZero();
}


void Drives::PortWrite(word addr, byte val)
{
    UNREFERENCED_PARAMETER(addr);

    ctrl_drive = getBits(val, cDriveSelectOfs, cDriveSelectBits);
    ctrl_side = getBit(val, cSideSelectOfs);
    ctrl_ddense = getBit(val, cDensitySelectOfs) != 0;
}


byte Drives::PortRead(word addr)
{
    UNREFERENCED_PARAMETER(addr);

    if (fdc->GetIntRQ() || fdc->GetDRQ())
        return cFDC_RQ_Flag;
    else
        return 0;
}


/*! \throws OutOfRange if \p drive does not specify a valid drive
 *  \throws DiskImageError if disk image \p name could not be loaded
 */
void Drives::LoadDisk(unsigned int drive, const char *name)
{
    if (drive >= cNumDrives)
        throw OutOfRange();

    UnloadDisk(drive);  // In case a disk is already loaded
    disks[drive] = new Disk(name);
}


void Drives::UnloadDisk(unsigned int drive)
{
    if (drive >= cNumDrives)
        throw OutOfRange();

    if (disks[drive] != NULL)
    {
        delete disks[drive];
        disks[drive] = NULL;
    }
}


void Drives::SetCylinder(unsigned int cyl_)
{
    if (cyl_ > cMaxCylinder)
        cyl[ctrl_drive] = cMaxCylinder;
    else
        cyl[ctrl_drive] = cyl_;
}


bool Drives::DiskLoaded() const
{
    return disks[ctrl_drive] != NULL;
}


bool Drives::DiskProtected() const
{
    if (DiskLoaded())
        return disks[ctrl_drive]->IsProtected();
    else
        return true;
}


bool Drives::TrackZero() const
{
    return cyl[ctrl_drive] == 0;
}


bool Drives::IndexHoleVisible() const
{
    return !DiskLoaded();
}


bool Drives::Ready() const
{
    return true;
}


void Drives::SeekTrackZero()
{
    SetCylinder(0);
}


void Drives::Step(int amt)
{
    SetCylinder(cyl[ctrl_drive] + amt);
}


/*! \param buf   Buffer to store the data read (must be at least as big as the current disk's sector size)
    \param sect  Physical sector to read

    The drive will attempt to read the sector specified from the current drive/side/cylinder.
 */
bool Drives::ReadSector(unsigned char *buf, unsigned int sect)
{
    if (DiskLoaded())
        return disks[ctrl_drive]->ReadSector(buf, ctrl_side, cyl[ctrl_drive], sect);
    else
        return false;
}


/*! \param buf   Buffer with the data to write (must be at least as big as the current disk's sector size)
    \param sect  Physical sector to write

    The drive will attempt to write the sector specified to the current drive/side/cylinder.
 */
bool Drives::WriteSector(unsigned char *buf, unsigned int sect)
{
    if (DiskLoaded())
        return disks[ctrl_drive]->WriteSector(buf, ctrl_side, cyl[ctrl_drive], sect);
    else
        return false;
}


bool Drives::ReadIDField(unsigned char *buf)
{
    if (DiskLoaded())
        return disks[ctrl_drive]->ReadIDField(buf, ctrl_side, cyl[ctrl_drive]);
    else
        return false;
}


bool Drives::FormatTrack(DSK_FORMAT *format, byte filler, unsigned int num_sectors)
{
    if (DiskLoaded())
        return disks[ctrl_drive]->FormatTrack(format, filler, num_sectors, ctrl_side, cyl[ctrl_drive]);
    else
        return false;
}