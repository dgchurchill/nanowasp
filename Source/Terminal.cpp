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
#include "Terminal.h"


BEGIN_EVENT_TABLE(Terminal, wxWindow)
    EVT_KEY_DOWN(Terminal::OnKeyDown)
    EVT_KEY_UP(Terminal::OnKeyUp)
END_EVENT_TABLE()


Terminal::Terminal(wxWindow *parent) : 
    wxGLCanvas(parent, wxID_ANY, NULL, wxDefaultPosition, wxSize(width, height))
{
    memset(keys, 0, cMaxKeyCode + 1);
}


Terminal::~Terminal()
{
}


void Terminal::OnKeyDown(wxKeyEvent &evt)
{
    keys[evt.GetKeyCode()] = true;
}


void Terminal::OnKeyUp(wxKeyEvent &evt)
{
    keys[evt.GetKeyCode()] = false;
}
