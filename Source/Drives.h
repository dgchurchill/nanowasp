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

#ifndef DRIVES_H
#define DRIVES_H

#include "PortDevice.h"
#include <vector>
#include <libdsk.h>

class Microbee;
class Disk;
class FDC;


/*! \brief Emulates the disk drives
 *
 *  This class is used to keep track of the status of the disk <em>drives</em>.
 *  See the Disk class for the emulation of the disks.  Specifically, this
 *  device emulates the flip-flops used to select the current drive, head, and
 *  disk density.  It also keeps track of what Disk (if any) is in each drive,
 *  and the current cylinder for each drive.  Finally, the device is used to
 *  pass status signals from the FDC back to the processor.
 */
class Drives : public PortDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    Drives(Microbee &mbee_, const TiXmlElement &config_);
    ~Drives();

    virtual void LateInit();


    virtual void Reset();

    virtual void PortWrite(word addr, byte val);
    virtual byte PortRead(word addr);

    //! Loads a disk from file \p name into \p drive
    void LoadDisk(unsigned int drive, const char *name);
    //! Removes the dsik from \p drive
    void UnloadDisk(unsigned int drive);

    //! Returns true if the current drive is empty, or if the loaded disk is write-protected
    bool DiskProtected() const;
    //! Returns true if the current drive is over cylinder zero
    bool TrackZero() const;
    //! Returns true if the current disk has the index hole at the indexing position
    bool IndexHoleVisible() const;
    //! Returns true if the current drive is ready
    bool Ready() const;

    //! Moves the current drive's head to cylinder zero
    void SeekTrackZero();
    //! Moves the current drive's head by (signed) \p amt cylinders
    void Step(int amt);

    //! Reads an ID field from the currently loaded disk
    bool ReadIDField(unsigned char *buf);
    //! Reads a sector from the currently loaded disk
    bool ReadSector(unsigned char *buf, unsigned int sect);
    //! Writes a sector to the currently loaded disk
    bool WriteSector(unsigned char *buf, unsigned int sect);
    //! Formats the current track
    bool FormatTrack(DSK_FORMAT *format, byte filler, unsigned int num_sectors);


    //! Returns the current drive's cylinder position
    unsigned int GetTrack() const { return cyl[ctrl_drive]; }
    //! Returns the current side selection
    byte GetSide() const { return ctrl_side; }


private:
    Microbee &mbee;  //!< Owning Microbee
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config

    FDC *fdc;  //!< Connection to FDC device, used to get status signals IntRQ and DRQ

    unsigned int ctrl_side;  //!< Currently selected side
    unsigned int ctrl_drive;  //!< Currently selected drive
    bool ctrl_ddense;  //!< Currently selected density

    std::vector<Disk*> disks;  //!< Disks for each drive
    std::vector<unsigned int> cyl;   //!< Current cylinder position for each drive


    const static unsigned int cNumDrives = 4;  //!< Total drives supported
    const static byte cMaxCylinder = 39;  //!< Maximum cylinder
    const static byte cNumPorts = 1;
    const static byte cDriveSelectBits = 2;
    const static byte cDriveSelectOfs = 0;
    const static byte cSideSelectOfs = 2;
    const static byte cDensitySelectOfs = 3;
    const static byte cFDC_RQ_Flag = 0x80;


    //! Set the cylinder position of the current drive
    void SetCylinder(unsigned int cyl_);

    //! Returns true if the current drive has a disk loaded
    bool DiskLoaded() const;

    // Private copy constuctor and assigment operator to prevent copies
    Drives(const Drives &);
    Drives& operator= (const Drives &);
};


#endif // DRIVES_H