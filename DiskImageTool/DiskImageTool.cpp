
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/stdpaths.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <wx/filename.h>
#include <sstream>
#include <iomanip>

#include "version.h"
#include "Forms.h"
#include "CPMFileSys.h"


class MainWindow : public MainWindowForm
{
public:
    MainWindow();
    ~MainWindow();

    virtual void OnOpen(wxCommandEvent& evt);
    virtual void OnExit(wxCommandEvent& evt);

    virtual void OnDelete(wxCommandEvent& evt);

    virtual void OnRename(wxCommandEvent& evt);
    virtual void OnEditLabelEnd(wxListEvent& evt);
    virtual void OnListSort(wxListEvent& evt);

    virtual void OnAbout(wxCommandEvent& evt);

    void OnDragItem(wxListEvent& evt);


private:
    CPMFileSys *cpmfs;
    bool read_only;  // Current disk is being treated as read-only (cached when opening new file)
    std::vector<CPMFileSys::DirEntry> dir;

    enum ListColumns
    {
        eName = 0,  // Name is assumed to be first element in RefreshList()
        eSize,
        eType,
        eUserArea
    };

    long sort_col;
    bool sort_reversed;


    void OpenImage(const char *name);
    void RefreshList();
    void DoSort();
    void DeleteSelected();

    static int wxCALLBACK listCompare(long item1, long item2, long col);
    static int wxCALLBACK listCompareReverse(long item1, long item2, long col);


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
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, MainWindow::OnEditLabelEnd)
    EVT_LIST_COL_CLICK(wxID_ANY, MainWindow::OnListSort)
END_EVENT_TABLE()



MainWindow::MainWindow() :
    MainWindowForm(NULL, wxID_ANY, APPNAME),
    cpmfs(NULL),
    read_only(true),
    sort_col(eName),
    sort_reversed(false)
{
    // Icon
    SetIcon(wxIcon("DiskImageToolIcon", wxBITMAP_TYPE_ICO_RESOURCE));


    // List
    m_listCtrl->InsertColumn(eName, "Name");
    m_listCtrl->SetColumnWidth(eName, 100);

    wxListItem li;
    li.SetText("Size");
    li.SetAlign(wxLIST_FORMAT_RIGHT);
    li.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT);
    m_listCtrl->InsertColumn(eSize, li);
    m_listCtrl->SetColumnWidth(eSize, 75);

    m_listCtrl->InsertColumn(eType, "Type");
    m_listCtrl->SetColumnWidth(eType, 125);

    li.Clear();
    li.SetText("User Area");
    li.SetAlign(wxLIST_FORMAT_RIGHT);
    li.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT);
    m_listCtrl->InsertColumn(eUserArea, li);
    m_listCtrl->SetColumnWidth(eUserArea, 75);
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


void MainWindow::OnDelete(wxCommandEvent& WXUNUSED(evt))
{
    int selected = m_listCtrl->GetSelectedItemCount();
    if (selected == 0)
        return;

    if (read_only)
    {
        wxMessageBox("Disk image is read-only.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    long first = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    struct CPMFileSys::DirEntry *dirent = (struct CPMFileSys::DirEntry *)(m_listCtrl->GetItemData(first));

    wxString conf_msg;
    if (selected == 1)
        conf_msg = "Are you sure you want to delete '" + dirent->name + "'?";
    else
        conf_msg = "Are you sure you want to delete these " + wxString::Format("%i", selected) + " items?";
    if (wxMessageBox(conf_msg, "Confirm File Delete", wxYES_NO | wxICON_EXCLAMATION) != wxYES)
        return;

    DeleteSelected();
}


void MainWindow::DeleteSelected()
{
    if (m_listCtrl->GetSelectedItemCount() == 0 || read_only)
        return;


    struct CPMFileSys::DirEntry *dirent;
    long item = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    try
    {
        while (item != -1)
        {
            dirent = (struct CPMFileSys::DirEntry *)(m_listCtrl->GetItemData(item));
            cpmfs->Delete(dirent->realname.c_str());
            item = m_listCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }
    }
    catch (CPMFileSys::GeneralError &ex)
    {
        wxMessageBox(wxString("Failed to delete file ") + dirent->name + wxString(".  ") + ex.what(), "Error", wxOK | wxICON_ERROR);
    }

    RefreshList();
}


void MainWindow::OnRename(wxCommandEvent& WXUNUSED(evt))
{
    if (read_only)
    {
        wxMessageBox("Disk image is read-only.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    long focused = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
    if (focused == -1)
        return;

    m_listCtrl->EditLabel(focused);

    // Actual renaming occurs in OnEditLabelEnd()
}


void MainWindow::OnEditLabelEnd(wxListEvent& evt)
{
    if (evt.IsEditCancelled() || read_only)
        return;

    // new name
    wxFileName fname(evt.GetLabel());
    size_t namelen = fname.GetName().Len();
    size_t extlen = fname.GetExt().Len();
    if (namelen > 8 || extlen > 3 || namelen + extlen == 0)
    {
        wxMessageBox("Filename must be in 8.3 format", "Error", wxOK | wxICON_ERROR);
        evt.Veto();
        return;
    }

    wxString dest = "00" + fname.GetName() + "." + fname.GetExt();
    struct CPMFileSys::DirEntry *dirent = (struct CPMFileSys::DirEntry *)(m_listCtrl->GetItemData(evt.GetIndex()));
    try
    {
        cpmfs->Rename(dirent->realname.c_str(), dest.c_str());
    }
    catch (CPMFileSys::GeneralError &ex)
    {
        wxMessageBox(wxString("Failed to rename file ") + dirent->name + wxString(".  ") + ex.what(), "Error", wxOK | wxICON_ERROR);
    }

    RefreshList();
}


void MainWindow::OnListSort(wxListEvent& evt)
{
    if (evt.GetColumn() == -1)
        return;  // Click was outside all headers

    if (evt.GetColumn() == sort_col)
    {
        sort_reversed = !sort_reversed;
    }
    else
    {
        sort_col = evt.GetColumn();
        sort_reversed = false;
    }

    DoSort();
}


void MainWindow::DoSort()
{
    if (sort_reversed)
        m_listCtrl->SortItems(MainWindow::listCompareReverse, sort_col);
    else
        m_listCtrl->SortItems(MainWindow::listCompare, sort_col);
}


int wxCALLBACK MainWindow::listCompareReverse(long item1, long item2, long col)
{
    return -listCompare(item1, item2, col);
}


int wxCALLBACK MainWindow::listCompare(long item1, long item2, long col)
{
    struct CPMFileSys::DirEntry *dirent1 = (struct CPMFileSys::DirEntry *)item1;
    struct CPMFileSys::DirEntry *dirent2 = (struct CPMFileSys::DirEntry *)item2;

    switch (col)
    {
    case eName:
        return wxString(dirent1->name).CmpNoCase(dirent2->name);
        break;

    case eSize:
        return (int)(dirent1->size - dirent2->size);
        break;

    case eType:
        return wxFileName(dirent1->name).GetExt().CmpNoCase(wxFileName(dirent2->name).GetExt());
        break;

    case eUserArea:
        return dirent1->user - dirent2->user;
        break;
    }

    return 0;
}



void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(evt))
{
    wxAboutDialogInfo info;

    info.SetIcon(wxIcon("DiskImageToolIcon", wxBITMAP_TYPE_ICO_RESOURCE));
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
    read_only = cpmfs->IsReadOnly();
    SetTitle(wxFileName(name).GetFullName() + (read_only ? " (Read-only) - " : " - ") + APPNAME);

    sort_col = eName;
    sort_reversed = false;
    RefreshList();

    m_listCtrl->SetDropTarget(read_only ? NULL : new DropTarget(*this));
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
            m_listCtrl->SetItem(ind, eSize, ss.str());

            wxString ext = wxFileName(dir[i].name).GetExt().Upper();
            wxString type_desc;
            if (ext == "MWB")
                type_desc = "MicroWorld Basic File";
            else if (ext == "COM")
                type_desc = "Command File";
            else
                type_desc = ext + " File";
            m_listCtrl->SetItem(ind, eType, type_desc);

            ss.str("");
            ss << dir[i].user;
            m_listCtrl->SetItem(ind, eUserArea, ss.str());

            m_listCtrl->SetItemPtrData(ind, (wxUIntPtr)&dir[i]);
        }
    }

    DoSort();

    struct CPMFileSys::CPMFSStat stat;
    cpmfs->GetStat(stat);
    GetStatusBar()->SetStatusText(wxString::Format("Disk space free: %i KB  (%i KB total)", stat.free/1024, stat.size/1024));
}


void MainWindow::OnDragItem(wxListEvent& WXUNUSED(evt))
{
    if (m_listCtrl->GetSelectedItemCount() == 0)
        return;


    // Create a temporary directory where the files can be extracted without overwriting anything
    wxString temp_dir;
    wxString base_dirname = wxStandardPaths::Get().GetTempDir() + wxFILE_SEP_PATH + APPNAME + 
                            wxString::Format("-%lu-", wxGetProcessId()) + wxDateTime::Now().Format("%Y%m%d%H%M%S");
    const int max_retries = 10;
    int i = 0;
    while (true)
    {
        temp_dir = base_dirname + wxString::Format("-%i", i);
        if (wxMkdir(temp_dir, 0700))
            break;

        ++i;
        if (i >= max_retries)
        {
            wxMessageBox("Unable to create temporary directory " + temp_dir, "Error", wxOK | wxICON_ERROR);
            return;
        }
    }


    // Copy out the selected items to the temp directory, and build a list of the files for the wxDropSource
    wxFileDataObject fdo;
    wxArrayString files;
    long item = -1;
    CPMFileSys::DirEntry *de;
    try
    {
        while ((item = m_listCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
        {
            de = (struct CPMFileSys::DirEntry *)(m_listCtrl->GetItemData(item));
            wxString dest = temp_dir + wxFILE_SEP_PATH + de->name;
            cpmfs->CopyFromCPM(de->realname.c_str(), dest.c_str());

            fdo.AddFile(dest);
        }

        files = fdo.GetFilenames();  // Create a local copy of the names in case wxDropSource does something funky with fdo


        // Do the drag
        wxDropSource source(fdo, this);
        wxDragResult result = source.DoDragDrop(wxDrag_CopyOnly);

      // TODO: Allow move when drag and dropping (because the files are extracted to
      //       a temporary directory, the shell optimises the move, so the return code is
      //       wxDragNone instead of wxDragMove).

    /*    if (result == wxDragMove)
            DeleteSelected(); */
    }
    catch (CPMFileSys::GeneralError &)
    {
        // this error dialog will kill the drag event...
        wxMessageBox("Unable to create temporary copy of file " + de->name, "Error", wxOK | wxICON_ERROR);

        files = fdo.GetFilenames(); // prepare for the cleanup code to be executed
    }


    // Clean up the temp files and directory
    for (size_t i = 0; i < files.GetCount(); ++i)
        wxRemoveFile(files[i]);
    wxRmdir(temp_dir);
}


bool MainWindow::DropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString& filenames)
{
    if (mw.cpmfs == NULL)
        return false;


    try
    {
        for (unsigned int i = 0; i < filenames.GetCount(); ++i)
        {
            wxFileName fname(filenames[i]);

            wxString dest = fname.GetName().Left(8) + "." + fname.GetExt().Left(3);
            wxString dest_real = "00" + dest;

            if (mw.cpmfs->Exists(dest_real.c_str()))
            {
                if (wxMessageBox("This disk image already contains a file named '" + dest + "'.  " 
                                 "Would you like to replace the existing file?", 
                                 "Confirm File Replace", wxYES_NO, &mw) == wxYES)
                {
                    mw.cpmfs->Delete(dest_real.c_str());
                }
                else
                    continue;
            }

            mw.cpmfs->CopyToCPM(filenames[i].c_str(), dest_real.c_str());
        }
    }
    catch (CPMFileSys::GeneralError &)
    {
        wxMessageBox("Failed to copy file to disk image (disk full?).", "Error", wxOK | wxICON_ERROR);
        mw.RefreshList();
        return false;
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

