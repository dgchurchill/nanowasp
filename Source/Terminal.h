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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <wx/glcanvas.h>
#include <vector>


/*! \brief Provides a frame for display of OpenGL graphics and captures key events for the emulator */
class Terminal : public wxGLCanvas
{
public:
    const static int width = 640;
    const static int height = 400;

    Terminal(wxWindow *parent);
    ~Terminal();

    void OnKeyDown(wxKeyEvent &evt);
    void OnKeyUp(wxKeyEvent &evt);

    //! Returns true if the key \p keycode (wxWidgets specific) is currently pressed
    bool IsPressed(const int keycode) const { return keys[keycode]; }


private:
    static const unsigned int cMaxKeyCode = WXK_COMMAND;  // TODO: This is OK using wxWidgets v2.8.4...
    bool keys[cMaxKeyCode + 1];  //!< Current status of keys, using a plain old array for speed


    DECLARE_EVENT_TABLE()
};


#endif // TERMINAL_H