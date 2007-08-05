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

#ifndef FDC_H
#define FDC_H

#include <vector>
#include "PortDevice.h"
#include <libdsk.h>

class Microbee;
class Drives;


/*! \brief Emulates the 2793 Floppy %Disk Controller
 *
 *  Commands which read/write multi-byte data are implemented using an internal buffer to collect
 *  the data.  Write commands are only executed once all the data has been received.  This is unlike the
 *  real FDC which will send the data to the write head as it is received.
 *
 *  \todo Support for non-standard geometries
 */
class FDC : public PortDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    FDC(Microbee &mbee_, const TiXmlElement &config_);

    virtual void LateInit();


    virtual void Reset();

    virtual void PortWrite(word addr, byte val);
    virtual byte PortRead(word addr);

    //! Returns the current IntRQ signal
    bool GetIntRQ();
    //! Returns the current DRQ signal
    bool GetDRQ();


private:
    Microbee &mbee;  //!< Owning Microbee
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config
    Drives *drives;  //!< Connects to the Drives device

    byte rcmd;     //!< Last/current command
    byte rdata;    //!< Data register
    byte rtrack;   //!< Track register
    byte rsect;    //!< Sector register
    byte rstatus;  //!< Status register

    bool intrq;
    bool drq;
    bool head_loaded;
    bool type1status;  //!< If true, status reflects Type I command, otherwise Type II/III


    enum
    {
        cOut = -1,
        cIn = 1
    } stepdir;   //!< Current step direction


    std::vector<byte> buf;  //!< Internal buffer for multi-byte commands
    std::vector<byte>::iterator buf_index;   //!< Position in buffer for current multi-byte command
    unsigned int bytes_left;  //!< Data bytes remaining for the current multi-byte command

    byte wt_filler;  //!< Write Track data area filler byte
    std::vector<DSK_FORMAT> wt_format; //!< Write Track formatting data
    enum
    {
        sWTGap,
        sWTMark,
        sWTIDFieldTrack,
        sWTIDFieldSide,
        sWTIDFieldSector,
        sWTIDFieldSize,
        sWTDataField
    } wt_state;  //!< Write track state
    int wt_secpertrack;  //!< Sectors per track


    Microbee::time_t emu_time;  //!< Time that the FDC system has been updated to
    void Update();  //!< Updates the system to the current time, called on any 

    void LogCommand();
    void LogStatus();

    enum
    {
        sIdle,

        sT1Init,
        sT1UpdateTrack,
        sT1CheckTrack0,
        sT1Step,
        sT1StepLoop,
        sT1Verify,

        sT2Init,
        sT2MainLoop,
        sT2Read,
        sT2ReadLoop,
        sT2Write,
        sT2Write2,
        sT2WriteLoop,
        sT2Finish,

        sT3Init,
        sT3ReadAddr,
        sT3ReadAddrLoop,
        sT3WriteTrack,
        sT3WriteTrack2,
        sT3WriteTrackLoop,
    } state;  //!< Current state of FDC


    static const Microbee::time_t cByteTime = 160;  //!< Time to read/write a single byte in microseconds

    static const unsigned int cBytesPerTrack = 10000; //!< Number of raw bytes on a disk track

    /*! microseconds expended if the record doesn't exist
     *
     *  This should really be variable if the emulation was accurate, but so
     *  long as it's not too small things will work OK.  Key test - when the Microbee
     *  loads its system tracks on booting, it uses a read multiple sector command.  According
     *  to the data sheet, this will always set the record not found flag on completion (because
     *  that's how it determines completion).  However, the routine in the ROM BIOS will treat the
     *  read as a failure if it sees this set on completion of the command.  Everything works
     *  because the ROM BIOS sends a Force Interrupt command once it's read the number of bytes
     *  it wants, which should terminate the command before it has a chance to set the record
     *  not found status (because the FDC will spin the disk at least four times looking for 
     *  the next sector).
     *
     *  It also looks like this value can't be too large or some Microbee software (inc. CP/M)
     *  might hit one of its own timeouts.
     */
    static const Microbee::time_t cSeekNotFoundTime = cByteTime * 100;

    // Ports
    enum
    {
        cCmd = 0,
        cStatus = 0,
        cTrack,
        cSect,
        cData,
        cNumPorts
    };


    // Commands
    const static byte cRestore = 0;       // Type I
    const static byte cSeek = 1;
    const static byte cStep = 2;
    const static byte cStepIn = 4;
    const static byte cStepOut = 6;

    const static byte cReadSect = 8;      // Type II
    const static byte cWriteSect = 10;

    const static byte cReadAddr = 12;     // Type III
    const static byte cReadTrack = 14;
    const static byte cWriteTrack = 15;

    const static byte cInterrupt = 13;    // Type IV

    byte GetCommandCode(byte cmd);
    bool IsForceInt(byte cmd);
    int GetCommandType(byte cmd);


    const static Microbee::time_t cStepDelays[];


    // Flags
    const static byte cNotReady = 0x80;   // Status port bits
    const static byte cWrProt = 0x40;
    const static byte cHeadLoaded = 0x20;
    const static byte cRecType = 0x20;
    const static byte cSeekError = 0x10;
    const static byte cRecNotFound = 0x10;
    const static byte cCRCError = 0x08;
    const static byte cTrack0 = 0x04;
    const static byte cLostData = 0x04;
    const static byte cIndexPulse = 0x02;
    const static byte cDRQ = 0x02;
    const static byte cBusy = 0x01;

    const static byte cStepRate = 0x03;   // FDC command data bits
    const static byte cVerify = 0x04;
    const static byte cLoadHead = 0x08;
    const static byte cUpdateTrack = 0x10;
    const static byte cMultiSect = 0x10;
    const static byte cSide = 0x08;
    const static byte cDelay = 0x04;
    const static byte cCmpSide = 0x02;
    const static byte cDataMark = 0x01;
    const static byte cIntReady = 0x01;
    const static byte cIntNotReady = 0x02;
    const static byte cIntIndex = 0x04;
    const static byte cIntImmed = 0x08;
};


#endif // FDC_H