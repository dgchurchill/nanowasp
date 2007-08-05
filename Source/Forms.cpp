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


#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "Forms.h"

#include "version.h"

///////////////////////////////////////////////////////////////////////////

AboutBox::AboutBox( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("Images/nanowasp-logo.png"), wxBITMAP_TYPE_PNG ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_bitmap1, 0, wxALL, 0 );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, wxT("nanowasp " VERSION_STR "\n\n" COPYRIGHT "\n" CONTACT), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer11->Add( m_staticText17, 1, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL, 25 );
	
	bSizer10->Add( bSizer11, 0, wxEXPAND, 15 );
	
	m_staticText21 = new wxStaticText( this, wxID_ANY, wxT("This software depends on the libraries listed below.  Thanks to the authors of\nthese projects for making their work available.\n\nwxWidgets - http://www.wxwidgets.org/\nlibdsk - http://www.seasip.demon.co.uk/Unix/LibDsk/\nTinyXML - http://tinyxml.sourceforge.net/\n\nThe Z80 CPU emulation is based on the libz80 code, available at:\nhttp://libz80.sourceforge.net/\n\n\n\nThis program is free software: you can redistribute it and/or modify it under the\nterms of the GNU General Public License as published by the Free Software\nFoundation, either version 3 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY\nWARRANTY; without even the implied warranty of MERCHANTABILITY or \nFITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License\nfor more details.\n\nYou should have received a copy of the GNU General Public License along with\nthis program.  If not, see <http://www.gnu.org/licenses/>"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_staticText21, 0, wxALL, 15 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	m_button1 = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_button1, 0, wxALL, 10 );
	
	bSizer10->Add( bSizer14, 1, wxALIGN_RIGHT, 5 );
	
	this->SetSizer( bSizer10 );
	this->Layout();
	bSizer10->Fit( this );
}
