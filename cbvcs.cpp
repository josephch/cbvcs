/*  cbvcs Code::Blocks version control system plugin

    Copyright (C) 2011 Dushara Jayasinghe.

    cbvcs is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cbvcs is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cbvcs.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include <cbproject.h>
#include <cbeditor.h>
#include <projectmanager.h>
#include <cbfunctor.h>
#include <logmanager.h>

#include "cbvcs.h"
#include "IVersionControlSystem.h"
#include "VcsFileItem.h"
#include "treeitemvector.h"
#include "vcsprojecttracker.h"
#include "vcstrackermap.h"
#include "shellutilimpl.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
PluginRegistrant<cbvcs> reg(_T("cbvcs"));
const int idAdd = wxNewId();
const int idRemove = wxNewId();
const int idCommit = wxNewId();
const int idDiff = wxNewId();
const int idRevert = wxNewId();
const int idRefresh = wxNewId();
#if 0
const int idBranchCreate = wxNewId();
const int idBranchCheckout = wxNewId();
const int idBranchMerge = wxNewId();
const int idTagCreate = wxNewId();
const int idTagCheckout = wxNewId();
#endif
}


// events handling
BEGIN_EVENT_TABLE(cbvcs, cbPlugin)
    // add any events you want to handle here
    EVT_MENU( idAdd, cbvcs::OnAdd )
    EVT_MENU( idRemove, cbvcs::OnRemove )
    EVT_MENU( idCommit, cbvcs::OnCommit )
    EVT_MENU( idDiff, cbvcs::OnDiff )
    EVT_MENU( idRevert, cbvcs::OnRevert )
    EVT_MENU( idRefresh, cbvcs::OnRefresh )
END_EVENT_TABLE()

// constructor
cbvcs::cbvcs()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("cbvcs.zip")))
    {
        NotifyMissingFile(_T("cbvcs.zip"));
    }

    m_ShellUtils = new ShellUtilImpl;
}

// destructor
cbvcs::~cbvcs()
// TODO (dushara#1#): Delete all vcs instances{
    delete m_ShellUtils;
}

void cbvcs::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_OPEN, new cbEventFunctor<cbvcs, CodeBlocksEvent>(this, &cbvcs::OnProjectOpen));
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_CLOSE, new cbEventFunctor<cbvcs, CodeBlocksEvent>(this, &cbvcs::OnProjectClose));
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_SAVE, new cbEventFunctor<cbvcs, CodeBlocksEvent>(this, &cbvcs::OnProjectSave));
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_SAVE, new cbEventFunctor<cbvcs, CodeBlocksEvent>(this, &cbvcs::OnEditorSave));
}

void cbvcs::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
}

int cbvcs::Configure()
{
    //create and display the configuration dialog for your plugin
    cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Your dialog title"));
    cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
    if (panel)
    {
        dlg.AttachConfigurationPanel(panel);
        PlaceWindow(&dlg);
        return dlg.ShowModal() == wxID_OK ? 0 : -1;
    }
    return -1;
}

void cbvcs::BuildMenu(wxMenuBar* menuBar)
{
    //The application is offering its menubar for your plugin,
    //to add any menu items you want...
    //Append any items you need in the menu...
    //NOTE: Be careful in here... The application's menubar is at your disposal.
    NotImplemented(_T("cbvcs::BuildMenu()"));
}

void cbvcs::CreateProjectMenu(wxMenu* menu, const FileTreeData* data)
{
    wxMenu* VcsMenu = new wxMenu(_("Git"));

#if 0
    wxMenu* branch = new wxMenu(_("Branch"));
    branch->Append(idBranchCreate, _("Create"), _("Create branch"));
    branch->Append(idBranchCheckout, _("Checkout"), _("Checkout branch"));
    branch->Append(idBranchMerge, _("Merge"), _("Merge branches"));

    wxMenu* tag = new wxMenu(_("Tag"));
    tag->Append(idTagCreate, _("Create"), _("Create a new tag"));
    tag->Append(idTagCheckout, _("Checkout"), _("Checkout a tag"));
#endif

    VcsMenu->Append(idCommit, _("Commit"), _("Commit this file"));
    VcsMenu->Append(idDiff, _("Diff"), _("View diff"));
    VcsMenu->Append(idRevert, _("Revert"), _("Revert changes"));
    VcsMenu->Append(idRefresh, _("Refresh"), _("Refresh VCS status"));

#if 0
    VcsMenu->AppendSubMenu(branch, _("Branch"));
    VcsMenu->AppendSubMenu(tag, _("Tag"));
#endif
    menu->AppendSubMenu(VcsMenu, _("Git"));
}

void cbvcs::CreateFolderMenu(wxMenu* menu)
{
    wxMenu* VcsMenu = new wxMenu(_("Git"));

    VcsMenu->Append(idAdd, _("Add"), _("Add this file"));
    VcsMenu->Append(idRemove, _("Remove"), _("Remove this file"));
    VcsMenu->Append(idCommit, _("Commit"), _("Commit this file"));
    VcsMenu->Append(idDiff, _("Diff"), _("View diff"));
    VcsMenu->Append(idRevert, _("Revert"), _("Revert changes"));
    VcsMenu->Append(idRefresh, _("Refresh"), _("Refresh VCS status"));

    menu->AppendSubMenu(VcsMenu, _("Git"));
}

void cbvcs::CreateFileMenu(wxMenu* menu, const FileTreeData* data)
{
    ProjectFile* file = data->GetProjectFile();

    if(file->GetFileState() == (FileVisualState)Item_UntrackedMissing )
    {
        return;
    }

    wxMenu* VcsMenu = new wxMenu(_("Git"));

    if(file->GetFileState() == (FileVisualState)Item_Untracked )
    {
        VcsMenu->Append(idAdd, _("Add"), _("Add this file"));
    }
    else if(file->GetFileState() == (FileVisualState)Item_Added
       || file->GetFileState() == (FileVisualState)Item_Removed)
    {
        VcsMenu->Append(idCommit, _("Commit"), _("Commit this file"));
        VcsMenu->Append(idRevert, _("Revert"), _("Revert changes"));
    }
    else if(file->GetFileState() == (FileVisualState)Item_UpToDate
       || file->GetFileState() == (FileVisualState)Item_Modified)
    {
        VcsMenu->Append(idRemove, _("Remove"), _("Remove this file"));

        if(file->GetFileState() == (FileVisualState)Item_Modified)
        {
            VcsMenu->Append(idCommit, _("Commit"), _("Commit this file"));
            VcsMenu->Append(idRevert, _("Revert"), _("Revert changes"));
            VcsMenu->Append(idDiff, _("Diff"), _("View diff"));
        }
    }
    else if(file->GetFileState() == (FileVisualState)Item_Missing)
    {
        VcsMenu->Append(idRemove, _("Remove"), _("Remove this file"));
        VcsMenu->Append(idRevert, _("Revert"), _("Revert changes"));
    }
    VcsMenu->Append(idRefresh, _("Refresh"), _("Refresh VCS status"));

    menu->AppendSubMenu(VcsMenu, _("Git"));
}

void cbvcs::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    if ( !menu || !IsAttached() || !data)
        return;

    if(type != mtProjectManager)
    {
        return;
    }

    if(data->GetKind() == FileTreeData::ftdkFile)
    {
        CreateFileMenu(menu, data);
    }
    else if(data->GetKind() == FileTreeData::ftdkFolder)
    {
        CreateFolderMenu(menu);
        // same menu as File but maybe traverse the tree
    }
    else if(data->GetKind() == FileTreeData::ftdkProject)
    {
        CreateProjectMenu(menu, data);
    }
}

bool cbvcs::BuildToolBar(wxToolBar* toolBar)
{
    //The application is offering its toolbar for your plugin,
    //to add any toolbar items you want...
    //Append any items you need on the toolbar...
    NotImplemented(_T("cbvcs::BuildToolBar()"));

    // return true if you add toolbar items
    return false;
}

vcsProjectTracker* cbvcs::GetVcsInstance(const FileTreeData *data)
{
    if ( !data )
        return 0;

    cbProject* prj = data->GetProject();
    if(!prj)
    {
        return 0;
    }

    const wxString& prj_file = prj->GetFilename();

    return m_ProjectTrackers.GetTracker(prj_file);
}

void cbvcs::GetFileItem(TreeItemVector& treeVector, const wxTreeCtrl& tree, const wxTreeItemId& treeItem)
{
    FileTreeData* ftd = static_cast<FileTreeData*>( tree.GetItemData( treeItem ) );

    if(ftd->GetKind() == FileTreeData::ftdkFile)
    {
        treeVector.CreateFileItem(ftd->GetProjectFile());
    }
}

void cbvcs::GetDescendents(TreeItemVector& treeVector, const wxTreeCtrl& tree, const wxTreeItemId& start)
{
    wxTreeItemIdValue cookie;

    if(!tree.ItemHasChildren(start))
    {
        return ;
    }

    wxTreeItemId child = tree.GetFirstChild(start, cookie);
    while(child.IsOk())
    {
        if(tree.ItemHasChildren(child))
        {
            GetDescendents(treeVector, tree, child);
        }
        else
        {
            GetFileItem(treeVector, tree, child);
        }
        child = tree.GetNextChild(start, cookie);
    }
}

enum  cbvcs::VcsAction : unsigned int
{
    VcsAction_Add,
    VcsAction_Remove,
    VcsAction_Commit,
    VcsAction_Diff,
    VcsAction_Revert,
    VcsAction_Refresh,
};

void cbvcs::PerformGroupActionOnSelection(VcsAction action)
{
    const wxTreeCtrl* tree;
    wxArrayTreeItemIds treeItems;
    vcsProjectTracker* selectedProjectTracker = nullptr;

    tree = Manager::Get()->GetProjectManager()->GetUI().GetTree();
    if(!tree)
    {
        return;
    }

    size_t selectionSize = tree->GetSelections(treeItems);
    if ( !selectionSize )
    {
        return;
    }
    TreeItemVector files;

    for (size_t i = 0; i < selectionSize; i++)
    {
        const wxTreeItemId& selItem = treeItems[i];
        FileTreeData* fileTreeData = static_cast<FileTreeData*>( tree->GetItemData( selItem ) );
        if ( !fileTreeData )
        {
            fprintf(stderr, "%s:%d : fileTree data not available. index %zu\n", __FUNCTION__, __LINE__, i);
            continue;
        }
        vcsProjectTracker* prjTracker = GetVcsInstance(fileTreeData);
        if(!prjTracker)
        {
            fprintf(stderr, "%s:%d : prjTracker data not available. index %zu\n", __FUNCTION__, __LINE__, i);
            continue;
        }
        selectedProjectTracker =  prjTracker;

        const FileTreeData::FileTreeDataKind fileTreeDataKind = fileTreeData->GetKind();
        if(fileTreeDataKind == FileTreeData::ftdkFile)
        {
            files.CreateFileItem(fileTreeData->GetProjectFile());
        }
        else if(fileTreeDataKind == FileTreeData::ftdkFolder)
        {
            GetDescendents(files, *tree, selItem);
        }
        else if(fileTreeDataKind == FileTreeData::ftdkProject)
        {
            cbProject* prj = fileTreeData->GetProject();
            files.CreateProjectItem(prj->GetFilename(), prjTracker->GetProjectState());
            GetDescendents(files, *tree, selItem);
        }
    }

    if(!selectedProjectTracker)
    {
        fprintf(stderr, "%s:%d : selectedProjectTracker  not available.\n", __FUNCTION__, __LINE__);
        return;
    }

    IVersionControlSystem& vcs = selectedProjectTracker->GetVcs();
    switch (action)
    {
    case VcsAction_Add:
        vcs.AddOp.execute(files.GetVector());
        break;
    case VcsAction_Remove:
        vcs.RemoveOp.execute(files.GetVector());
        break;
    case VcsAction_Commit:
        vcs.CommitOp.execute(files.GetVector());
        break;
    case VcsAction_Diff:
        vcs.DiffOp.execute(files.GetVector());
        break;
    case VcsAction_Revert:
        vcs.RevertOp.execute(files.GetVector());
        break;
    case VcsAction_Refresh:
        vcs.UpdateOp.execute(files.GetVector());
        break;
    }
    vcs.UpdateOp.execute(files.GetVector());
}

void cbvcs::OnAdd( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Add);
}

void cbvcs::OnRemove( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Remove);
}

void cbvcs::OnCommit( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Commit);
}

void cbvcs::OnDiff( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Diff);
}

void cbvcs::OnRevert( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Revert);
}

void cbvcs::OnRefresh( wxCommandEvent& /*event*/ )
{
    PerformGroupActionOnSelection(VcsAction_Refresh);
}

void cbvcs::OnProjectOpen( CodeBlocksEvent& event )
{
    cbProject* prj = event.GetProject();

    if(!prj)
    {
        return;
    }

    const wxString prjFilename = prj->GetFilename();

    if(!m_ProjectTrackers.CreateTracker(prjFilename, *m_ShellUtils))
    {
        return;
    }

    vcsProjectTracker* prjTracker;
    prjTracker = m_ProjectTrackers.GetTracker(prjFilename);

    if(!prjTracker)
    {
        // Impossible really cos we just created it!
        return;
    }

    IVersionControlSystem& vcs = prjTracker->GetVcs();

    TreeItemVector files;
    files.CreateProjectItem(prjFilename, prjTracker->GetProjectState());

    for ( int i = 0; i < prj->GetFilesCount(); ++i )
    {
        ProjectFile* pf = prj->GetFile( i );
        files.CreateFileItem(pf);
    }

    vcs.UpdateOp.execute(files.GetVector());
}

void cbvcs::OnProjectClose( CodeBlocksEvent& event )
{
    cbProject* prj = event.GetProject();

    if(!prj)
    {
        return;
    }

    const wxString prj_file = prj->GetFilename();
    m_ProjectTrackers.RemoveTracker(prj_file);
}

void cbvcs::OnProjectSave( CodeBlocksEvent& event )
{
    cbProject* prj = event.GetProject();

    if(!prj)
    {
        Manager::Get()->GetLogManager()->Log( _("null") );
        return;
    }

    const wxString prjFilename = prj->GetFilename();

    vcsProjectTracker* prjTracker = m_ProjectTrackers.GetTracker(prjFilename);
    if(!prjTracker)
    {
        // uncontrolled project
        return;
    }
    TreeItemVector files;
    files.CreateProjectItem(prjFilename, prjTracker->GetProjectState());
    prjTracker->GetVcs().UpdateOp.execute(files.GetVector());
}

void cbvcs::OnEditorSave( CodeBlocksEvent& event )
{
    cbEditor* ed = (cbEditor*) event.GetEditor();

    if(!ed)
    {
        Manager::Get()->GetLogManager()->Log(_("Editor NULL"));
        return;
    }

    cbProject* prj = Manager::Get()->GetProjectManager()->GetActiveProject();

    if(!prj)
    {
        Manager::Get()->GetLogManager()->Log(_("Prj NULL"));
        return;
    }

    ProjectFile* SavedFile = prj->GetFileByFilename(ed->GetFilename(), false, true);
    if(!SavedFile)
    {
        // File not part of project. Just ignore
        Manager::Get()->GetLogManager()->Log(ed->GetFilename() + _(" Not in project"));
        return;
    }

    const wxString& prjFilename = prj->GetFilename();

    vcsProjectTracker* prjTracker = m_ProjectTrackers.GetTracker(prjFilename);
    if(!prjTracker)
    {
        // Project not tracked
        return;
    }

    std::vector<VcsTreeItem*>UpdateList;

    VcsFileItem vcsItem(SavedFile);
    UpdateList.push_back(&vcsItem);
    prjTracker->GetVcs().UpdateOp.execute(UpdateList);
}
