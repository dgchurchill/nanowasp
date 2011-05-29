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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>

#include <wx/wx.h>

#include "Exceptions.h"


#ifndef UNREFERENCED_PARAMETER
#    ifdef __WXOSX__
#        define UNREFERENCED_PARAMETER(P)
#    else
#        define UNREFERENCED_PARAMETER(P) (P)
#    endif
#endif


// TODO: Move these to a better place?
typedef unsigned char byte;
typedef unsigned short word;


// IDs
enum
{
    ID_Reset = 1,
    ID_Pause,
    ID_Resume,
    ID_LoadDiskA,
    ID_LoadDiskB,
    ID_CreateDisk,
    ID_SaveState,
};


inline word getBits(word val, byte ofs, byte num) { return (val >> ofs) % (1 << num); }
inline word getBit(word val, byte ofs) { return (val >> ofs) & 1; }

inline word clearBits(word val, byte ofs, byte num) { return val & ~(((1 << num) - 1) << ofs); }
inline word clearBit(word val, byte ofs) { return val & ~(1 << ofs); }

inline word setBits(word val, byte ofs, byte num) { return val | (((1 << num) - 1) << ofs); }
inline word setBit(word val, byte ofs) { return val | (1 << ofs); }

inline word copyBits(word old, byte ofs, byte num, word val) { return clearBits(old, ofs, num) | (getBits(val, 0, num) << ofs); }
inline word copyBit(word old, byte ofs, word val) { return clearBit(old, ofs) | ((val & 1) << ofs); }
