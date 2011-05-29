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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class Microbee;
class Terminal;


/*! \brief Implements the main window
 *
 */
class MainWindow : public wxFrame
{
public:
    //! Constructs the main window and initiates an instance of the emulator thread
    MainWindow();

    //! Set the focus to the terminal when activated
    void Activate(wxActivateEvent& evt);

    //! Handles an exit command from the user
    void OnExit(wxCommandEvent& evt);
    //! Displays the "About" dialog
    void OnAbout(wxCommandEvent& evt);
    //! Shuts down the emulator gracefully
    void OnClose(wxCloseEvent& evt);

    //! Saves the emulator state
    void OnSaveState(wxCommandEvent& evt);
    
    //! Loads a disk into the drive
    void OnLoadDiskA(wxCommandEvent& evt);
    void OnLoadDiskB(wxCommandEvent& evt);
    //! Pauses the emulation
    void OnPause(wxCommandEvent& evt);
    //! Resumes the emulation
    void OnResume(wxCommandEvent& evt);
    //! Resets the emulator
    void OnReset(wxCommandEvent& evt);

    //! Creates a blank floppy image
    void CreateDisk(wxCommandEvent& evt);


private:
    Microbee *mbee;  //!< The emulated machine
    Terminal *term;  //!< The terminal

    static const wxString app_name;

    DECLARE_EVENT_TABLE()
};

#endif // MAINWINDOW_H