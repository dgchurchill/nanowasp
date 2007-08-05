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
#include "FDC.h"

#include <iostream>

#include "Microbee.h"
#include "Drives.h"
#include "Disk.h"


const Microbee::time_t FDC::cStepDelays[] = {3000, 6000, 10000, 15000};  // In microseconds


/*! \p config_ must contain a <connect> to the associated Drives device. */
FDC::FDC(Microbee &mbee_, const TiXmlElement &config_) :
    PortDevice(cNumPorts),
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    drives(NULL)
{
}


void FDC::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "FDC <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "Drives")
            drives = mbee.GetDevice<Drives>(dest);
    }

    if (drives == NULL)
        throw ConfigError(&xml_config, "FDC missing Drives connection");
}


void FDC::Reset()
{
    // Conceivably an Update() should be performed here (at least for resets that occur after the system has run)
    emu_time = mbee.GetTime();
    state = sIdle;
    head_loaded = false;
    drq = intrq = false;
    type1status = true;
    rstatus &= ~cBusy;
    stepdir = cIn;
    rsect = 1;
    rcmd = 0;
    PortWrite(cCmd, 0x03);  // Restore, slowest stepping (this is executed on reset according to the specs)
}


void FDC::PortWrite(word addr, byte val)
{
    Update();

    switch (addr % cNumPorts)
    {
    case cCmd:
        rcmd = val;

/*        LogStatus();
        LogCommand();  */

        if ((rstatus & cBusy) && !IsForceInt(rcmd))
            break;   // Ignore any commands other than force interrupt if already processing (actual behaviour is not defined in datasheet)


        switch (GetCommandType(rcmd))
        {
        case 1:
            state = sT1Init;
            type1status = true;
            break;

        case 2:
            state = sT2Init;
            type1status = false;
            break;

        case 3:
            state = sT3Init;
            type1status = false;
            break;

        case 4:
            // Force Interrupt
            if ((rstatus & cBusy) == 0)
                type1status = true;  // If Force Interrupt is received when not busy, status reflects Type I status

            state = sIdle;
            rstatus &= ~cBusy;
            drq = intrq = false;

            // TODO: Implement IntRQ generation
            break;
        }

        break;


    case cTrack:
        rtrack = val;
        break;


    case cSect:
        rsect = val;
        break;


    case cData:
        drq = false;
        rdata = val;
        break;
    }
}


byte FDC::PortRead(word addr)
{
    Update();

    switch (addr % cNumPorts)
    {
    case cStatus:
        intrq = false;

        if (type1status)
        {
            // Returning Type I status

            // Clear the bits which come from external signals
            rstatus &= ~(cNotReady | cWrProt | cTrack0 | cIndexPulse);

            // Set them according to the external signals
            if (!drives->Ready())
                rstatus |= cNotReady;

            if (drives->DiskProtected())
                rstatus |= cWrProt;

            if (head_loaded)
                rstatus |= cHeadLoaded;

            if (drives->TrackZero())
                rstatus |= cTrack0;

            if (drives->IndexHoleVisible())
                rstatus |= cIndexPulse;
        }
        else
        {
            // Returning Type II/III status

            // Clear the bits which come from external signals
            rstatus &= ~(cNotReady | cDRQ);

            // Set them according to the external signals
            if (!drives->Ready())
                rstatus |= cNotReady;

            if (drq)
                rstatus |= cDRQ;
        }

        return rstatus;
        break;


    case cTrack:
        return rtrack;
        break;


    case cSect:
        return rsect;
        break;


    case cData:
        drq = false;
        return rdata;
        break;


    default:
        return 0;
    }
}


bool FDC::GetIntRQ()
{ 
    Update();
    return intrq; 
}


bool FDC::GetDRQ()
{
    Update();
    return drq; 
}


void FDC::Update()
{
    // microseconds to run for.  Should run for as close to this time as possible, without
    // exceeding it (if it does exceed it then, e.g., the CPU may lose data even
    // though it really would have read it in time).
    Microbee::time_t now = mbee.GetTime();
    Microbee::time_t run_time = now - emu_time;

    byte cmd_code = GetCommandCode(rcmd);

    unsigned char id_field[6], first_id[6];
    byte tmp;

    bool found;

    bool done = false;
    while (!done)
    {
        switch (state)
        {
        case sIdle:
            run_time = 0;  // Idled to a stop
            done = true;
            break;


        ///////////////////////////////////////////////
        //              Type I Commands              //
        ///////////////////////////////////////////////
        case sT1Init:
            // State execution time = 0us

            rstatus |= cBusy;
            rstatus &= ~(cCRCError | cSeekError);
            intrq = drq = false;

            // Head load timing isn't emulated
            head_loaded = (rcmd & cLoadHead) != 0;

            switch (cmd_code)
            {
            case cStepIn:
            case cStepOut:
                if (cmd_code == cStepIn)
                    stepdir = cIn;
                else
                    stepdir = cOut;

                // fall through
            case cStep:
                if (rcmd & cUpdateTrack)
                    state = sT1UpdateTrack;
                else
                    state = sT1CheckTrack0;
                break;


            case cRestore:
                rtrack = 255;
                rdata = 0;

                // fall through
            default:
                // seek or restore
                if (rdata > rtrack)
                    stepdir = cIn;
                else
                    stepdir = cOut;

                state = sT1StepLoop;
                break;
            }
            break;


        case sT1UpdateTrack:
            // State execution time = 0us
            rtrack += stepdir;
            state = sT1CheckTrack0;
            break;

        case sT1CheckTrack0:
            // State execution time = 0us
            if (stepdir == cOut && drives->TrackZero())
            {
                rtrack = 0;
                state = sT1Verify;
            }
            else
                state = sT1Step;
            break;

        case sT1Step:
            // State execution time dependent on command parameter
            if (run_time - cStepDelays[rcmd & cStepRate] < 0)
            {
                done = true;
                break;
            }
            run_time -= cStepDelays[rcmd & cStepRate];

            drives->Step(stepdir);

            switch (cmd_code)
            {
            case cStepIn:
            case cStepOut:
            case cStep:
                state = sT1Verify;
                break;

            default:
                state = sT1StepLoop;
                break;
            }
            break;

        case sT1StepLoop:
            // State execution time = 0us
            if (rtrack == rdata)
                state = sT1Verify;
            else
                state = sT1UpdateTrack;
            break;

        case sT1Verify:
            // State execution time = 0us (time to find address mark / timeout is not emulated)
            intrq = true;
            rstatus &= ~cBusy;
            state = sIdle;

            if ((rcmd & cVerify) == 0)
                break;

            head_loaded = true;
            
            // Timeout not implemented (retry until 5 index holes passed)
            // CRC error check on address field not implemented

            // Scan all the ID fields to see if one claims to be on the track we want
            rstatus |= cSeekError;
            if (drives->ReadIDField(id_field))
            {
                first_id[0] = id_field[0];
                first_id[1] = id_field[1];
                first_id[2] = id_field[2];

                do
                {
                    if (id_field[0] == rtrack)
                    {
                        rstatus &= ~cSeekError;
                        break;
                    }
                }
                while (drives->ReadIDField(id_field) && 
                       (id_field[0] != first_id[0] || 
                        id_field[1] != first_id[1] || 
                        id_field[2] != first_id[2])
                      );  // TODO: This could be improved
            }

            break;


        ///////////////////////////////////////////////
        //             Type II Commands              //
        ///////////////////////////////////////////////
        case sT2Init:
            // State execution time = 0us
            rstatus |= cBusy;
            rstatus &= ~(cLostData | cRecNotFound | cRecType | cWrProt);  // TODO: This should also clear CRC error?
            drq = false;

            if (!drives->Ready())
            {
                intrq = true;
                rstatus &= ~cBusy;
                state = sIdle;
                break;
            }

            head_loaded = true;
            state = sT2MainLoop;
            break;

        case sT2MainLoop:
            // State execution time is variable
            if (cmd_code == cWriteSect && drives->DiskProtected())
            {
                intrq = true;
                rstatus |= cWrProt;
                rstatus &= ~cBusy;
                state = sIdle;
            }

            // Scan for the sector we're trying to work with
            found = false;
            if (drives->ReadIDField(id_field))
            {
                first_id[0] = id_field[0];
                first_id[1] = id_field[1];
                first_id[2] = id_field[2];

                do
                {
                    if (id_field[0] == rtrack &&
                        id_field[2] == rsect &&
                        ((rcmd & cCmpSide) == 0 || id_field[1] == getBit(rcmd, 3)))
                    {
                        found = true;
                        break;
                    }
                }
                while (drives->ReadIDField(id_field) &&
                       (id_field[0] != first_id[0] || 
                        id_field[1] != first_id[1] || 
                        id_field[2] != first_id[2])
                      );  // TODO: This could be improved
            }

            if (!found)
            {
                // No sector found, so we should spend cSeekNotFoundTime in this state
                // 'found' should probably be cached.  This is NQR because the write
                // protect check shouldn't be made each time through.
                if (run_time - cSeekNotFoundTime < 0)
                {
                    done = true;
                    break;
                }
                run_time -= cSeekNotFoundTime;

                intrq = true;
                rstatus |= cRecNotFound;
                rstatus &= ~cBusy;
                state = sIdle;
                break;
            }

            bytes_left = Disk::SectorTypeToSize(id_field[3]);
            if (bytes_left == 0)
            {
                // Something's not quite right if we end up here, but handle it nicely
                state = sT2Finish;
                break;
            }
            if (buf.size() < bytes_left)
                buf.resize(bytes_left);
            buf_index = buf.begin();

            if (cmd_code == cWriteSect)
                state = sT2Write;
            else
                state = sT2Read;
            break;

        case sT2Read:
            // State execution time = 0us
            drives->ReadSector(&buf[0], rsect);  // TODO: Extend this to read deleted / non-deleted status and set status bit accordingly
            state = sT2ReadLoop;
            break;

        case sT2ReadLoop:
            // State execution time = cByteTime (TODO: this should change when in single density...)
            if (run_time - cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= cByteTime;

            if (drq)  // last byte wasn't read
                rstatus |= cLostData;

            rdata = *buf_index;
            ++buf_index;
            drq = true;
            if (--bytes_left)
                break;  // state remains sT2ReadLoop

            // TODO: Check CRC
            
            state = sT2Finish;
            break;

        case sT2Write:
            // State execution time = 2*cByteTime (TODO: this should change when in single density...)
            if (run_time - 2*cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= 2*cByteTime;

            drq = true;
            state = sT2Write2;
            break;

        case sT2Write2:
            // State execution time = 8*cByteTime (TODO: this should change when in single density...)
            if (run_time - 8*cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= 8*cByteTime;

            if (drq) // data register wasn't loaded
            {
                intrq = true;
                rstatus |= cLostData;
                rstatus &= ~cBusy;
                state = sIdle;
            }
            else
                state = sT2WriteLoop;
            break;

        case sT2WriteLoop:
            // State execution time = cByteTime (TODO: this should change when in single density...)
            if (run_time - cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= cByteTime;

            if (drq)  // DRQ was not serviced
                *buf_index = 0;
            else
                *buf_index = rdata;
            ++buf_index;

            drq = true;

            if (--bytes_left)
                break;  // state remains sT2WriteLoop


            drives->WriteSector(&buf[0], rsect);
            state = sT2Finish;
            break;

        case sT2Finish:
            // State execution time = 0us
            if (rcmd & cMultiSect)
            {
                ++rsect;
                state = sT2MainLoop;
                break;
            }

            intrq = true;
            rstatus &= ~cBusy;
            state = sIdle;
            break;


        ///////////////////////////////////////////////
        //             Type II Commands              //
        ///////////////////////////////////////////////

        case sT3Init:
            rstatus |= cBusy;
            rstatus &= ~(cLostData | cRecNotFound | cRecType);

            if (!drives->Ready())
            {
                intrq = true;
                rstatus &= ~cBusy;
                state = sIdle;
                break;
            }

            head_loaded = true;


            switch (cmd_code)
            {
            case cReadAddr:
                state = sT3ReadAddr;
                break;

            case cReadTrack:
                state = sIdle;
                // TODO: Implement
                break;

            case cWriteTrack:
                state = sT3WriteTrack;
                break;
            }
            break;

        case sT3ReadAddr:
            bytes_left = 6;
            if (buf.size() < bytes_left)
                buf.resize(bytes_left);
            buf_index = buf.begin();

            if (drives->ReadIDField(&buf[0]))
            {
                state = sT3ReadAddrLoop;
                rsect = buf[0];
                break;
            }
            else
            {
                intrq = true;
                rstatus &= ~cBusy;
                rstatus |= cRecNotFound;
                state = sIdle;
                break;
            }
            break;

        case sT3ReadAddrLoop:
            // State execution time = cByteTime (TODO: this should change when in single density...)
            if (run_time - cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= cByteTime;

            // TODO: Check if this is supposed to set data lost or not

            rdata = *buf_index;
            ++buf_index;
            drq = true;
            if (--bytes_left)
                break;  // state remains sT3ReadAddrLoop

            // TODO: Check CRC
            
            intrq = true;
            rstatus &= ~cBusy;
            state = sIdle;
            break;

        case sT3WriteTrack:
            if (drives->DiskProtected())
            {
                intrq = false;
                rstatus &= ~cBusy;
                rstatus |= cWrProt;
                state = sIdle;
                break;
            }

            drq = true;

            state = sT3WriteTrack2;
            break;

        case sT3WriteTrack2:
            if (run_time - 3*cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= 2*cByteTime;  // Last byte's worth of delay occurs in sT2WriteTrackLoop

            if (drq)  // if not serviced
            {
                intrq = true;
                rstatus |= cLostData;
                rstatus &= ~cBusy;
                state = sIdle;
                break;
            }

            bytes_left = cBytesPerTrack;
            wt_filler = 0;
            wt_state = sWTGap;
            wt_format.clear();

            state = sT3WriteTrackLoop;
            break;

        case sT3WriteTrackLoop:
            if (run_time - cByteTime < 0)
            {
                done = true;
                break;
            }
            run_time -= cByteTime;

            if (drq) // if not serviced
                tmp = 0;
            else
                tmp = rdata;


            // Assumption: The write track command only ever writes filler bytes to
            // the sector data areas (it can't write general data, because of the
            // special interpretation by the FDC for address marks, etc).  The last
            // data byte written is used to fill all sectors.

            // TODO: Implement single density
            switch (tmp)
            {
            case 0xF5:
                // 0xA1 with missing clock, init CRC
                wt_state = sWTMark;
                break;

            case 0xF6:
                // 0xC2 with missing clock (track start)
                break;

            case 0xF7:
                // Write CRC
                wt_state = sWTGap;
                break;

            default:
                // Normal byte
                switch (wt_state)
                {
                case sWTMark:
                    if (tmp == 0xFE)
                    {
                        wt_format.push_back(DSK_FORMAT());
                        wt_format.back().fmt_cylinder = 0;
                        wt_format.back().fmt_head = 0;
                        wt_format.back().fmt_sector = 0;
                        wt_format.back().fmt_secsize = 0;
                        wt_state = sWTIDFieldTrack;
                    }
                    else if (tmp == 0xFB)  // TODO: Also check for 0xF8? (deleted data mark)
                        wt_state = sWTDataField;
                    else
                        wt_state = sWTGap;  // Ignore the unknown mark
                    break;

                case sWTIDFieldTrack:
                    wt_format.back().fmt_cylinder = tmp;
                    wt_state = sWTIDFieldSide;
                    break;

                case sWTIDFieldSide:
                    wt_format.back().fmt_head = tmp;
                    wt_state = sWTIDFieldSector;
                    break;

                case sWTIDFieldSector:
                    wt_format.back().fmt_sector = tmp;
                    wt_state = sWTIDFieldSize;
                    break;

                case sWTIDFieldSize:
                    wt_format.back().fmt_secsize = Disk::SectorTypeToSize(tmp);
                    wt_state = sWTGap;  // Really expecting CRC now...
                    break;

                case sWTDataField:
                    wt_filler = tmp;
                    break;
                }
                break;
            }

            if (--bytes_left == 0)
            {
                // .size() is zero, e.g., when the software is trying to determine the number of bytes / track, or erase the old track
                if (wt_format.size() == 0)
                    drives->FormatTrack(NULL, wt_filler, 0);
                else
                    drives->FormatTrack(&wt_format[0], wt_filler, (unsigned int)wt_format.size());
                intrq = true;
                rstatus &= ~cBusy;
                state = sIdle;
                break;
            }
            // otherwise state in current state

            break;
        }
    }


    emu_time = now - run_time;
}


byte FDC::GetCommandCode(byte cmd)
{
    if ((getBit(cmd, 7) ^ getBit(cmd, 6)) == 0)
        return getBits(cmd, 4, 4);
    else
        return getBits(cmd, 4, 4) & ~1;  // In this case the low order bit is actually a parameter to the command
}


bool FDC::IsForceInt(byte cmd)
{
    return getBits(cmd, 4, 4) == 13; 
}


int FDC::GetCommandType(byte cmd)
{
    switch (getBits(cmd, 6, 2))
    {
    case 0:
    case 1:
        return 1;
        break;

    case 2:
        return 2;
        break;

    case 3:
        if (IsForceInt(cmd))
            return 4;
        else
            return 3;
        break;

    default:
        // should be unreachable
        return 0;
    }
}


void FDC::LogCommand()
{
    using namespace std;

    cout << mbee.GetTime() << "  " << hex << showbase << (int)rcmd << dec << " (";

    switch (GetCommandCode(rcmd))
    {
    case cRestore:
        cout << "Restore";
        break;
    case cSeek:
        cout << "Seek";
        break;
    case cStep:
        cout << "Step";
        break;
    case cStepIn:
        cout << "StepIn";
        break;
    case cStepOut:
        cout << "StepOut";
        break;
    case cReadSect:
        cout << "ReadSect";
        break;
    case cWriteSect:
        cout << "WriteSect";
        break;
    case cReadAddr:
        cout << "ReadAddr";
        break;
    case cReadTrack:
        cout << "ReadTrack";
        break;
    case cWriteTrack:
        cout << "WriteTrack";
        break;
    case cInterrupt:
        cout << "Interrupt";
        break;
    }

    cout << "): Track = " << (int)rtrack << ", Sect = " << (int)rsect << ", Data = " << (int)rdata << "\n";
}


void FDC::LogStatus()
{
    using namespace std;

    cout << "Last status = ";

    if (rstatus & cNotReady)
        cout << "NotReady, ";
    if (rstatus & cWrProt)
        cout << "WrProt, ";
    if (rstatus & cCRCError)
        cout << "CRCError, ";
    if (rstatus & cBusy)
        cout << "Busy, ";

    if (type1status)
    {
        if (rstatus & cHeadLoaded)
            cout << "HeadLoaded, ";
        if (rstatus & cSeekError)
            cout << "SeekError, ";    
        if (rstatus & cTrack0)
            cout << "Track0, ";    
        if (rstatus & cIndexPulse)
            cout << "IndexPulse, ";    
    }
    else
    {
        if (rstatus & cRecType)
            cout << "RecType, ";
        if (rstatus & cRecNotFound)
            cout << "RecNotFound, ";
        if (rstatus & cLostData)
            cout << "cLostData, ";
        if (rstatus & cDRQ)
            cout << "DRQ, ";
    }

    cout << "\n";
}
