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
#include "Keyboard.h"

#include "Microbee.h"
#include "CRTC.h"
#include "Terminal.h"
#include "LatchROM.h"


/*! \p config_ must contain a <connect> to the associated CRTC and LatchROM devices. */
Keyboard::Keyboard(Microbee &mbee_, const TiXmlElement &config_) :
    mbee(mbee_),
    xml_config(config_),  // Create a local copy of the config
    config(&xml_config),
    term(mbee.GetTerminal()),
    crtc(NULL),
    latch_rom(NULL)
{
}


void Keyboard::LateInit()
{
    // Make connections
    for (TiXmlElement *el = xml_config.FirstChildElement("connect"); el != NULL; el = el->NextSiblingElement("connect"))
    {
        const char *type = el->Attribute("type");
        const char *dest = el->Attribute("dest");

        if (type == NULL || dest == NULL)
            throw ConfigError(el, "Keyboard <connect> missing type or dest attribute");

        std::string type_str = std::string(type);
        if (type_str == "CRTC")
            crtc = mbee.GetDevice<CRTC>(dest);
        else if (type_str == "LatchROM")
            latch_rom = mbee.GetDevice<LatchROM>(dest);
    }

    if (crtc == NULL)
        throw ConfigError(&xml_config, "Keyboard missing CRTC connection");
    if (latch_rom == NULL)
        throw ConfigError(&xml_config, "Keyboard missing LatchROM connection");
}


/*! This function assumes that key scanning is enabled (i.e. it ignores the latch ROM
    status on the assumption that RA4 is currently raised). */
void Keyboard::Check(word maddr)
{
   if (term.IsPressed(keymap[getBits(maddr, cKeyOfs, cKeyBits)]))
      crtc->TriggerLPen(maddr);
}


/*! This function takes into account the current status of the latch ROM, and only triggers
    the lightpen if this flip-flop is clear.  It assumes RA4 is low). */
void Keyboard::CheckAll()
{
   if (!latch_rom->GetLatch())
   {
      for (int i = cNumKeys - 1; i >= 0; --i)
      {
         if (term.IsPressed(keymap[i]))
         {
            crtc->TriggerLPen(i << cKeyOfs);
            break;
         }
      }
   }
}


const int Keyboard::keymap[] =
{
    '\'',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    '[',
    '\\',
    ']',
    '~',
    WXK_DELETE,
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    ';',
    '+',
    ',',
    '-',
    '.',
    '/',
    WXK_ESCAPE,
    WXK_BACK,
    WXK_TAB,
    WXK_NUMPAD_ENTER,     /* LF */
    WXK_RETURN,
    WXK_CAPITAL,
    WXK_NUMPAD_ADD,       /* break */
    WXK_SPACE,
    WXK_F1,               /* 61 */
    WXK_CONTROL,
    WXK_F2,               /* 62 */
    WXK_F5,               /* 65 */
    WXK_F4,               /* 64 */
    WXK_F3,               /* 63 */
    WXK_F6,               /* 66 */
    WXK_SHIFT
};
