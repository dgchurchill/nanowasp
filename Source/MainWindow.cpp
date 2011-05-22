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
#include "MainWindow.h"
#include <wx/stdpaths.h>

#include "Forms.h"

#include "Microbee.h"
#include "Terminal.h"
#include "Disk.h"



BEGIN_EVENT_TABLE(MainWindow, wxFrame)
  EVT_MENU(wxID_EXIT, MainWindow::OnExit)
  EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)

  EVT_MENU(ID_LoadDiskA, MainWindow::OnLoadDiskA)
  EVT_MENU(ID_LoadDiskB, MainWindow::OnLoadDiskB)
  EVT_MENU(ID_Pause, MainWindow::OnPause)
  EVT_MENU(ID_Resume, MainWindow::OnResume)
  EVT_MENU(ID_Reset, MainWindow::OnReset)

  EVT_MENU(ID_CreateDisk, MainWindow::CreateDisk)

  EVT_ACTIVATE(MainWindow::Activate)
  EVT_CLOSE(MainWindow::OnClose)
END_EVENT_TABLE()


const wxString MainWindow::app_name = _T("nanowasp");


MainWindow::MainWindow() :
    wxFrame(NULL, -1, MainWindow::app_name, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)),
    mbee(NULL)
{
    // Icons
  //  SetIcon(wxIcon("NanowaspIcon", wxBITMAP_TYPE_ICO_RESOURCE));

    // Menus
    wxMenuBar* menubar = new wxMenuBar();

    // File menu
    wxMenu* menu = new wxMenu();
    menu->Append(wxID_EXIT, _T("E&xit"));
    menubar->Append(menu, _T("&File"));

    // Microbee menu
    menu = new wxMenu();
    menu->Append(ID_LoadDiskA, _T("Load Disk &A"), "Loads a disk image into drive A");
    menu->Append(ID_LoadDiskB, _T("Load Disk &B"), "Loads a disk image into drive B");
    menu->AppendSeparator();
    menu->Append(ID_Pause, _T("&Pause"), "Pauses the emulation");
    menu->Append(ID_Resume, _T("&Resume"), "Resumes the emulation");
    menu->AppendSeparator();
    menu->Append(ID_Reset, _T("Reset"), "Resets the emulation");
    menubar->Append(menu, _T("&Microbee"));

    // Tools menu
    menu = new wxMenu();
    menu->Append(ID_CreateDisk, _T("&New disk..."));
    menubar->Append(menu, _T("&Tools"));

    // Help menu
    menu = new wxMenu();
    menu->Append(wxID_ABOUT, _T("&About"));
    menubar->Append(menu, _T("&Help"));

    SetMenuBar(menubar);


    // Status bar
    CreateStatusBar(1, wxFULL_REPAINT_ON_RESIZE);  // Don't show the resize gripper because we can't resize


    // Main client area
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    term = new Terminal(this);
    sizer->Add(term);
    SetSizer(sizer);
    sizer->Fit(this);


    // Start up the emulation.  This thread posts messages back to the windows, 
    // and so must be shut down before the windows are destroyed.  Thus the
    // Delete() call is made from this class, and for consistency the Create()
    // call is also made from here (as opposed to Nanowasp::OnInit()).
    try
    {
#ifdef __WXOSX__
        wxString xmlFile = wxStandardPaths::Get().GetResourcesDir() + "/Microbee.xml";
#else
        wxString xmlFile("Microbee.xml");
#endif
        
        mbee = new Microbee(*term, xmlFile.c_str());
        mbee->Create();
        mbee->Run();
    }
    catch (ConfigError &cfg_error)
    {
        wxMessageBox(cfg_error.what(), "Configuration Error", wxOK | wxICON_ERROR);
        Close(true);
    }
}


void MainWindow::Activate(wxActivateEvent &evt)
{
    term->SetFocus();
    evt.Skip();
}


void MainWindow::OnExit(wxCommandEvent& WXUNUSED(evt))
{
    Close(true);
}


void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(evt))
{
    AboutBox(this).ShowModal();
}


/* The event handler first deletes the emulator thread so that it doesn't
   attempt to reference any windows that might soon be deleted. */
void MainWindow::OnClose(wxCloseEvent& evt)
{
    if (mbee)
        mbee->Delete();

    evt.Skip();
}


void MainWindow::OnLoadDiskA(wxCommandEvent& WXUNUSED(evt))
{
    wxString filename = wxFileSelector("Select Disk Image for Drive A", wxGetCwd(), "", "", 
                              "DSK Files (*.dsk)|*.dsk", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (!filename.empty())
    {
        try
        {
            mbee->LoadDisk(0, filename.c_str());
        }
        catch (DiskImageError &)
        {
            wxMessageBox("Unable to load disk image " + filename, "Error", wxOK | wxICON_ERROR);
        }
    }
}


void MainWindow::OnLoadDiskB(wxCommandEvent& WXUNUSED(evt))
{
    wxString filename = wxFileSelector("Select Disk Image for Drive B", wxGetCwd(), "", "", 
                              "DSK Files (*.dsk)|*.dsk", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (!filename.empty())
    {
        try
        {
            mbee->LoadDisk(1, filename.c_str());
        }
        catch (DiskImageError &)
        {
            wxMessageBox("Unable to load disk image " + filename, "Error", wxOK | wxICON_ERROR);
        }
    }
}


void MainWindow::OnPause(wxCommandEvent& WXUNUSED(evt))
{
    mbee->PauseEmulation();
}


void MainWindow::OnResume(wxCommandEvent& WXUNUSED(evt))
{
    mbee->ResumeEmulation();
}


void MainWindow::OnReset(wxCommandEvent& WXUNUSED(evt))
{
    mbee->DoReset();
}


void MainWindow::CreateDisk(wxCommandEvent& WXUNUSED(evt))
{
    wxFileDialog file_dlg(this, "New Disk", wxGetCwd(), "", "DSK Files (*.dsk)|*.dsk", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (file_dlg.ShowModal() == wxID_OK)
    {
        try
        {
            Disk dsk(file_dlg.GetPath(), 2, 40, 10, 512);  // TODO: Don't specify constants here
            // Using the constructor above creates the file, which is all we want to do at the moment

            wxMessageBox("Blank disk image created successfully.", "Success", wxOK | wxICON_INFORMATION);
        }
        catch (DiskImageError &)
        {
            wxMessageBox("Creation of blank disk image failed.", "Error", wxOK | wxICON_ERROR);  // TODO: Add more detail...
        }
    }
}