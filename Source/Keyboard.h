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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Device.h"

class Terminal;
class CRTC;
class LatchROM;
class Microbee;


/*! \brief Emulates the keyboard
 *
 *  The Microbee's keyboard is connected to the system through the CRTC's light pen input.
 *  A demux/mux pair driven from the CRTC's memory address signals is used to route a
 *  single key switch at a time to the light pen.  As the CRTC scans the video ram to
 *  produce the output signal, it's also scanning the keyboard.  
 * 
 *  Scanning of the keyboard is only enabled if either the latch ROM is clear or signal RA4 from
 *  the CRTC is high.  Normally RA4 won't become high when scanning the screen(?), but the Microbee's
 *  CRTC has a custom function that allows a specified address to be gated on to the MA lines which also
 *  presumably raises the RA4 line.  This provides the software with a way to instantly check a given key's state
 *  (used to check the shift key after another key press has been detected, for example).
 *
 *
 *  \todo User-configurable key map
 */
class Keyboard : public Device
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    Keyboard(Microbee &mbee_, const TiXmlElement &config_);

    virtual void LateInit();


    //! Checks the status of the key at \p maddr, triggers the light pen if pressed
    void Check(word maddr);
    //! Checks the status of all keys, triggers the light pen for the first key found to be pressed
    void CheckAll();


private:
    Microbee &mbee;  //!< Owning Microbee
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config
    Terminal &term;  //!< Connection to terminal frame, used to get status of real keys
    CRTC *crtc;  //!< Connection to CRTC, for light pen signal
    LatchROM *latch_rom;  //!< Connection to LatchROM, used to disable normal keyboard scanning

    static const int keymap[];  //!< Mapping between real and emulated keys

    static const byte cNumKeys = 64;
    static const byte cKeyBits = 6;
    static const byte cKeyOfs = 4;
};

#endif // KEYBOARD_H