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
#include "CRTC.h"

#include <GL/glu.h>

#include "Terminal.h"
#include "CRTCMemory.h"
#include "Keyboard.h"


/*! \p config_ must contain a <connect> to the associated CRTCMemory and Keyboard devices. */
CRTC::CRTC(Microbee &mbee_, const TiXmlElement &config_) :
    PortDevice(cNumPorts),
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    crtc_mem(NULL),
    keyb(NULL),
    scr(mbee.GetTerminal()),
    gl_ctx(NULL)
{
}


CRTC::~CRTC()
{
    delete gl_ctx;
}


void CRTC::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "CRTC <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "CRTCMemory")
            crtc_mem = mbee.GetDevice<CRTCMemory>(dest);
        else if (type_str == "Keyboard")
            keyb = mbee.GetDevice<Keyboard>(dest);
    }

    if (crtc_mem == NULL)
        throw ConfigError(&xml_config, "CRTC missing CRTCMemory connection");
    if (keyb == NULL)
        throw ConfigError(&xml_config, "CRTC missing Keyboard connection");
}


void CRTC::Reset()
{
    reg = 0;
    hdisp = htot = 0;
    vdisp = vtot = 0;
    vtot_adj = 0;
    scans_per_row = 0;
    disp_start = 0;
    cur_pos = 0;
    mem_addr = 0;
    frame_counter = 0;
    cursor_on = false;
    blink_rate = 0;
    last_frame_time = emu_time = 0;
    frame_time = 1;
}


void CRTC::PortWrite(word addr, byte val)
{
    switch (addr % cNumPorts)
    {
    case cAddress:
        reg = val % cNumRegs;
        break;


    case cData:
        switch (reg)
        {
          case cReg_HTot:
             htot = val + 1;
             CalcVBlank();
             break;

          case cReg_HDisp:
             hdisp = val;
             break;


          case cReg_VTot:
             vtot = getBits(val, 0, 7) + 1;
             CalcVBlank();
             break;

          case cReg_VTotAdj:
             vtot_adj = getBits(val, 0, 5);
             CalcVBlank();
             break;

          case cReg_VDisp:
             vdisp = getBits(val, 0, 7);
             break;


          case cReg_Scanlines:
             scans_per_row = getBits(val, 0, 5) + 1;
             CalcVBlank();
             break;


          case cReg_CursorStart:
             cur_start = getBits(val, 0, 5);
             cur_mode = getBits(val, cBlinkOfs, 2);
             switch (cur_mode)
             {
             case cNoBlink:
                 cursor_on = true;
                 blink_rate = 0;
                 break;

             case cNoCursor:
                 cursor_on = false;
                 blink_rate = 0;
                 break;

             case cBlink16:
                 blink_rate = 16;
                 break;

             case cBlink32:
                 blink_rate = 32;
                 break;
             }
             break;

          case cReg_CursorEnd:
             cur_end = getBits(val, 0, 5);
             break;


          case cReg_DispStartH:
             disp_start = copyBits(disp_start, 8, 6, val);
             break;

          case cReg_DispStartL:
             disp_start = copyBits(disp_start, 0, 8, val);
             break;


          case cReg_CursorPosH:
             cur_pos = copyBits(cur_pos, 8, 6, val);
             break;

          case cReg_CursorPosL:
             cur_pos = copyBits(cur_pos, 0, 8, val);
             break;


          case cReg_SetAddrH:
             mem_addr = copyBits(mem_addr, 8, 6, val);
             break;

          case cReg_SetAddrL:
             mem_addr = copyBits(mem_addr, 0, 8, val);
             break;

          case cReg_DoSetAddr:
              keyb->Check(mem_addr);
             break;
        }
        break;
    }
}


byte CRTC::PortRead(word addr)
{
    byte status;

    switch (addr % cNumPorts)
    {
    case cStatus:
        status = cStatus_Strobe;    // Always set update strobe

        if (!lpen_valid)  // Don't check the keyboard if a keypress is already 'buffered'
            keyb->CheckAll();

        if (lpen_valid)  // keyb->CheckAll() might set lpen_valid
            status |= cStatus_LPen;

        if (mbee.GetTime() % frame_time < vblank_time)  // This is not quite correct, but probably good enough (GetTime() is absolute, frame_time may change, vblanking occurs at end of frame)
            status |= cStatus_VBlank;

        return status;
        break;


    case cData:
        switch (reg)
        {
          case cReg_CursorPosH:
             return getBits(cur_pos, 8, 6);
             break;

          case cReg_CursorPosL:
             return getBits(cur_pos, 0, 8);
             break;


          case cReg_LPenH:
             lpen_valid = false;
             return getBits(lpen, 8, 6);
             break;

          case cReg_LPenL:
             lpen_valid = false;
             return getBits(lpen, 0, 8);
             break;

          default:
             return 0xFF;
        }
        break;

    default:
        return 0xFF;
    }
}


Microbee::time_t CRTC::Execute(Microbee::time_t time, Microbee::time_t micros)
{
    emu_time = time + micros;  // Time to update to
    Microbee::time_t delta = emu_time - last_frame_time;

    if (delta >= frame_time)
    {
        Render(); // micros may be a long time, but no point rendering more than one frame

        frame_counter += delta / frame_time;
        last_frame_time = emu_time - delta % frame_time;  // The emulated time the frame really finished

        if (blink_rate > 0 && frame_counter > blink_rate)
        {
            if (((frame_counter / blink_rate) & 0x01) > 0)  // odd number of blinks (where a blink is on=>off or off=>on)
                cursor_on = !cursor_on;

            frame_counter %= blink_rate;
        }
    }

    return last_frame_time + frame_time - emu_time;
}


void CRTC::Render()
{
    if (gl_ctx == NULL)
    {
        gl_ctx = new wxGLContext(&scr);  // Here because it's guaranteed to be in the context of this thread
        gl_ctx->SetCurrent(scr);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-0.5, Terminal::width - 0.5, Terminal::height - 0.5, -0.5);
        glMatrixMode(GL_MODELVIEW);
        glViewport(0, 0, Terminal::width, Terminal::height);
        glColor3d(0.0, 1.0, 0.0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    word maddr = disp_start;

    for (word i = 0; i < vdisp; i++)
    {
        glRasterPos2i(0, (i+1) * scans_per_row - 1);
        for (word j = 0; j < hdisp; j++)
        {
            const unsigned char *bmp = crtc_mem->GetCharBitmap(maddr, scans_per_row);

            if (cursor_on && maddr == cur_pos)
            {
                unsigned char cursor_bmp[CRTCMemory::cBitmapSize];

                for (word k = 0; k < scans_per_row; k++)
                {
                    if ((scans_per_row - k - 1) >= cur_start && (scans_per_row - k - 1) <= cur_end)
                        cursor_bmp[k] = bmp[k] ^ 0xFF;
                    else
                        cursor_bmp[k] = bmp[k];
                }

                bmp = cursor_bmp;
            }

            glBitmap(cCharWidth, scans_per_row, 0.0, 0.0, cCharWidth, 0.0, bmp);

            maddr = (maddr + 1) % cMAddrSize;
        }
    }

    glFlush();
    scr.SwapBuffers();
}


void CRTC::TriggerLPen(word maddr)
{
   if (!lpen_valid)
   {
      lpen_valid = true;
      lpen = maddr;
   }
}


void CRTC::CalcVBlank()
{
    frame_time = (Microbee::time_t)htot * ((Microbee::time_t)vtot * scans_per_row + vtot_adj) * 1000000 / cCharClock;
    vblank_time = (Microbee::time_t)htot * (((Microbee::time_t)vtot - vdisp) * scans_per_row + vtot_adj) * 1000000 / cCharClock;

    if (frame_time == 0)
        frame_time = 1;  // Ensure we don't divide by zero later
}
