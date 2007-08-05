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
#include "Nanowasp.h"

#include "MainWindow.h"


/*! Creates the MainWindow and makes it visible */
bool Nanowasp::OnInit()
{
   // _CrtSetBreakAlloc(1536);

    // TODO: Implement proper logging?  At the moment this is here mainly to suppress
    //       wxLogGUI from popping dialogs randomly.
    //wxLog *old_log = wxLog::SetActiveTarget(new wxLogStderr(NULL));
    //delete old_log;
    wxInitAllImageHandlers();

    MainWindow *tf;
    tf = new MainWindow();
    tf->Show();
    SetTopWindow(tf);

    return true;
}


IMPLEMENT_APP(Nanowasp);

