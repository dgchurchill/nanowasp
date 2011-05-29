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

#ifndef CRTC_H
#define CRTC_H

#include "PortDevice.h"
#include <wx/glcanvas.h>
#include "Microbee.h"

class Microbee;
class Terminal;
class CRTCMemory;
class Keyboard;


/*! \brief Emulates the 6545 CRT Controller
 *
 *  The focus of the CRTC emulation is on the CPU-visible interface, and not
 *  the output signals used to drive the actual CRT.  The output signals
 *  that would have been generated are instead rendered directly to the screen.
 *
 *  \todo V-blanking status
 */
class CRTC : public PortDevice
{
public:
    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    CRTC(Microbee &mbee_, const TiXmlElement &config_);
    ~CRTC();

    virtual void LateInit();
    
    virtual void SaveState(BinaryWriter& writer);
    virtual void RestoreState(BinaryReader& reader);

    virtual void Reset();

    virtual void PortWrite(word addr, byte val);
    virtual byte PortRead(word addr);


    virtual Microbee::time_t Execute(Microbee::time_t time, Microbee::time_t micros);
    virtual bool Executable() { return true; }

    Microbee::time_t GetTime() { return emu_time; };  // CRTC doesn't provide fine-grained time emulation to other devices


    //! Returns the current Display Start value
    word GetDispStart() const { return disp_start; }
    //! Simulates triggering of the light pen input
    void TriggerLPen(word maddr);


private:
    Microbee &mbee; //!< Owning Microbee
    TiXmlElement xml_config;  //!< Configuration
    TiXmlHandle config;  //!< Handle to xml_config
    CRTCMemory *crtc_mem;  //!< Connection to the CRTCMemory device, used for character data
    Keyboard *keyb;  //!< Connection to the Keyboard device, to pass through requests from the CPU
    Terminal &scr;  //!< Terminal frame to render the display signal to
    wxGLContext *gl_ctx;  //!< OpenGL rendering context

    unsigned int frame_counter;  //!< Used for cursor blinking, number of frames since last cursor blink
    Microbee::time_t emu_time;  //!< Current emulated time (generally valid only for Execute() and GetTime())
    Microbee::time_t last_frame_time;  //!< Last emulated time a frame was rendered


    byte reg;  //!< Currently selected register

    word mem_addr;    //!< User set memory address (used by the Microbee for checking keyboard status)
    word disp_start;  //!< Display Start address

    word htot;           //!< Horizontal Total (for h-sync generation)
    word hdisp;          //!< Horizonal Displayed 
    word vtot;           //!< Verticial Total (for v-sync)
    word vtot_adj;       //!< Vertical Total Adjust (for v-sync)
    word vdisp;          //!< Vertical Displayed
    word scans_per_row;  //!< Scan lines per character row

    Microbee::time_t frame_time;   //!< Time each frame lasts for (microseconds)
    Microbee::time_t vblank_time;  //!< Time vblank is active for (microseconds)

    word cur_start;  //!< Cursor Start
    word cur_end;    //!< Cursor End
    word cur_mode;   //!< Cursor Mode
    word cur_pos;    //!< Cursor Position
    bool cursor_on;  //!< Cursor is currently visible
    word blink_rate;  //!< Number of frames per blink

    word lpen;  //!< Memory address of last light pen trigger
    bool lpen_valid;  //!< Light pen address is valid


    // Ports
    enum
    {
        cAddress = 0,
        cStatus = 0,
        cData,
        cNumPorts
    };

    // Registers
    enum
    {
        cReg_HTot = 0,
        cReg_HDisp,
        cReg_HSyncPos,
        cReg_SyncWidth,
        cReg_VTot,
        cReg_VTotAdj,
        cReg_VDisp,
        cReg_VSyncPos,
        cReg_Mode,
        cReg_Scanlines,
        cReg_CursorStart,
        cReg_CursorEnd,
        cReg_DispStartH,
        cReg_DispStartL,
        cReg_CursorPosH,
        cReg_CursorPosL,
        cReg_LPenH,
        cReg_LPenL,
        cReg_SetAddrH,
        cReg_SetAddrL,
        cReg_DoSetAddr = 31,
        cNumRegs
    };


    // Flags
    static const byte cStatus_Strobe = 0x80;
    static const byte cStatus_LPen = 0x40;
    static const byte cStatus_VBlank = 0x20;


    // Other
    static const int cCharWidth = 8;
    static const int cBlinkOfs = 5;
    static const int cMAddrSize = 16384;
    enum 
    {
        cNoBlink = 0,
        cNoCursor,
        cBlink16,
        cBlink32
    };
    static const unsigned long cCharClock = 1687500;  //!< Character clock frequency (Hz)


    //! Calculates helper variables for emulating the vertical blanking status
    void CalcVBlank();

    //! Renders the screen according to the current state
    void Render();

    // Private copy constuctor and assigment operator to prevent copies
    CRTC(const CRTC &);
    CRTC& operator= (const CRTC &);
};


#endif // CRTC_H