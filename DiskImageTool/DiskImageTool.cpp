
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/stdpaths.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <wx/filename.h>
#include <sstream>
#include <iomanip>

#include "Forms.h"
#include "CPMFileSys.h"


class MainWindow : public MainWindowForm
{
public:
    MainWindow();
    ~MainWindow();

    virtual void OnOpen(wxCommandEvent& evt);
    virtual void OnExit(wxCommandEvent& evt);

    virtual void OnCut(wxCommandEvent& evt);
    virtual void OnCopy(wxCommandEvent& evt);
    virtual void OnPaste(wxCommandEvent& evt);
    virtual void OnDelete(wxCommandEvent& evt);

    virtual void OnAbout(wxCommandEvent& evt);

    void OnDragItem(wxListEvent& evt);


private:
    CPMFileSys *cpmfs;
    std::vector<CPMFileSys::DirEntry> dir;

    void OpenImage(const char *name);
    void RefreshList();


    class DropTarget : public wxFileDropTarget
    {
    public:
        DropTarget(MainWindow &mw_) : mw(mw_) {}
        virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

    private:
        MainWindow &mw;
    };
    friend DropTarget;


    DECLARE_EVENT_TABLE();
};


BEGIN_EVENT_TABLE(MainWindow, MainWindowForm)
EVT_LIST_BEGIN_DRAG(ID_MainList, MainWindow::OnDragItem)
END_EVENT_TABLE()



MainWindow::MainWindow() :
    MainWindowForm(NULL),
    cpmfs(NULL)
{
    m_listCtrl->InsertColumn(0, "Name");
    m_listCtrl->SetColumnWidth(0, 200);

    wxListItem li;
    li.SetText("Size");
    li.SetAlign(wxLIST_FORMAT_RIGHT);
    li.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT);
    m_listCtrl->InsertColumn(1, li);
    m_listCtrl->SetColumnWidth(1, 100);

    li.Clear();
    li.SetText("User Area");
    li.SetAlign(wxLIST_FORMAT_RIGHT);
    li.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT);
    m_listCtrl->InsertColumn(2, li);
    m_listCtrl->SetColumnWidth(2, 100);

    SetDropTarget(new DropTarget(*this));
}


MainWindow::~MainWindow()
{
    if (cpmfs != NULL)
        delete cpmfs;
}


void MainWindow::OnOpen(wxCommandEvent& WXUNUSED(evt))
{
    wxString filename = wxFileSelector("Select Disk Image", NULL, NULL, NULL, 
                                       "DSK Files (*.dsk)|*.dsk", wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (!filename.empty())
        OpenImage(filename);
}


void MainWindow::OnExit(wxCommandEvent& WXUNUSED(evt))
{
    Close(true);
}


void MainWindow::OnCut(wxCommandEvent& evt)
{
}


void MainWindow::OnCopy(wxCommandEvent& evt)
{
}


void MainWindow::OnPaste(wxCommandEvent& evt)
{
}


void MainWindow::OnDelete(wxCommandEvent& evt)
{
}


void MainWindow::OnAbout(wxCommandEvent& evt)
{
    wxAboutDialogInfo info;

    info.SetName("Disk Image Tool");
    info.SetVersion("1.0");
    info.SetCopyright("Copyright © 2007 David G. Churchill");

    wxAboutBox(info);
}



void MainWindow::OpenImage(const char *name)
{
    if (cpmfs != NULL)
        delete cpmfs;

    cpmfs = new CPMFileSys(name);
    RefreshList();
}


void MainWindow::RefreshList()
{
    m_listCtrl->DeleteAllItems();

    if (cpmfs != NULL)
    {
        cpmfs->GetDirectory(dir);

        for (unsigned int i = 0; i < dir.size(); ++i)
        {
            long ind = m_listCtrl->InsertItem(0, dir[i].name);

            std::ostringstream ss;
            ss << dir[i].size / 1024 << " KB";
            m_listCtrl->SetItem(ind, 1, ss.str());

            ss.str("");
            ss << dir[i].user;
            m_listCtrl->SetItem(ind, 2, ss.str());

            m_listCtrl->SetItemData(ind, i);
        }
    }
}


void MainWindow::OnDragItem(wxListEvent& evt)
{
    if (m_listCtrl->GetSelectedItemCount() == 0)
        return;


    wxFileDataObject fdo;

    long item = -1;
    while ((item = m_listCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
    {
        CPMFileSys::DirEntry *de;

        de = &dir[m_listCtrl->GetItemData(item)];
        wxString dest = wxStandardPaths::Get().GetTempDir() + "\\" + de->name;
        cpmfs->CopyFromCPM(de->realname.c_str(), dest.c_str());

        fdo.AddFile(dest);
    }

    wxDropSource source(fdo, this);
    wxDragResult result = source.DoDragDrop();
}


bool MainWindow::DropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
{
    if (mw.cpmfs == NULL)
        return false;


    for (unsigned int i = 0; i < filenames.GetCount(); ++i)
    {
        wxFileName fname(filenames[i]);

        wxString dest = "00" + fname.GetName().Left(8) + "." + fname.GetExt().Left(3);
        mw.cpmfs->CopyToCPM(filenames[i].c_str(), dest.c_str());
    }

    mw.RefreshList();

    return true;
}


class DiskImageTool : public wxApp
{
    virtual bool OnInit();
};



bool DiskImageTool::OnInit()
{
    MainWindow *tf;
    tf = new MainWindow();
    tf->Show();
    SetTopWindow(tf);

    return true;
}


IMPLEMENT_APP(DiskImageTool);

