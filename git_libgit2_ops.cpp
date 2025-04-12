/*  cbvcs Code::Blocks version control system plugin

    Copyright (C) 2011 Dushara Jayasinghe.
    Copyright (C) 2024 Christo Joseph

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

#include "git_libgit2_ops.h"
#include "CommitMsgDialog.h"
#include "VcsTreeItem.h"
#include "git_libgit2_wrapper.h"
#include "icommandexecuter.h"
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include <editormanager.h>
#include <git2.h>
#include <manager.h>

ItemState getItemStateFromLibGit2StatusFlag(unsigned int statusFlags)
{
    ItemState status;

    if (statusFlags & GIT_STATUS_INDEX_NEW)
    {
        status = Item_Added;
    }
    else if (statusFlags & GIT_STATUS_INDEX_MODIFIED)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_INDEX_RENAMED)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_INDEX_TYPECHANGE)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_INDEX_DELETED)
    {
        status = Item_Removed;
    }
    else if (statusFlags & GIT_STATUS_CONFLICTED)
    {
        status = Item_Conflicted;
    }
    else if (statusFlags & GIT_STATUS_WT_NEW)
    {
        status = Item_Untracked;
    }
    else if (statusFlags & GIT_STATUS_WT_MODIFIED)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_WT_RENAMED)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_WT_TYPECHANGE)
    {
        status = Item_Modified;
    }
    else if (statusFlags & GIT_STATUS_WT_DELETED)
    {
        status = Item_Removed;
    }
    else if (statusFlags & GIT_STATUS_CONFLICTED)
    {
        status = Item_Conflicted;
    }
    else
    {
        status = Item_UpToDate;
    }
    return status;
}

LibGit2UpdateOp::LibGit2UpdateOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2_Op(vcs, vcsRootDir, shellUtils) {}

void LibGit2UpdateOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> proj_files)
{
    GitRepo gitRepo(m_VcsRootDir);
    fprintf(stderr, "LibGit2::%s:%d Enter. m_VcsRootDir %s proj_files size %zu\n", __FUNCTION__, __LINE__, m_VcsRootDir.ToUTF8().data(),
            proj_files.size());

    if (gitRepo.m_repo)
    {
        for (auto fi = proj_files.begin(); fi != proj_files.end(); fi++)
        {
            VcsTreeItem *pf = fi->get();

            wxString relativeFilename = pf->GetRelativeName(m_VcsRootDir);

            if (relativeFilename.length() == 0)
            {
                fprintf(stderr, "LibGit2::%s:%d couldn't get relativeFilename for vcsTreeItem %s\n", __FUNCTION__, __LINE__,
                        pf->GetName().ToUTF8().data());
                continue;
            }
            unsigned int statusFlags = 0;
            int error = git_status_file(&statusFlags, gitRepo.m_repo, relativeFilename.ToUTF8().data());
            if (0 != error)
            {
                const git_error *e = git_error_last();
                fprintf(stderr, "LibGit2::%s:%d git_status_file failed for file %s : %d/%d: %s\n", __FUNCTION__, __LINE__,
                        relativeFilename.ToUTF8().data(), error, e->klass, e->message);
                if (wxFileExists(relativeFilename))
                {
                    pf->SetState(Item_UpToDate);
                }
                else
                {
                    pf->SetState(Item_UntrackedMissing);
                }
            }
            else
            {
                fprintf(stderr, "LibGit2::%s:%d git_status_file file %s statusFlags 0x%x\n", __FUNCTION__, __LINE__, relativeFilename.ToUTF8().data(),
                        statusFlags);
                pf->SetState(getItemStateFromLibGit2StatusFlag(statusFlags));
            }
            pf->VisualiseState();
        }
    }
}

LibGit2UpdateFullOp::LibGit2UpdateFullOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2UpdateOp(vcs, vcsRootDir, shellUtils)
{
#ifdef TRACE
    fprintf(stderr, "LibGit2UpdateFullOp::%s:%d [%p]\n", __FUNCTION__, __LINE__, this);
#endif
}

struct VcsTreeItemAndRelativePath
{
    std::shared_ptr<VcsTreeItem> item;
    wxString relativeName;
    VcsTreeItemAndRelativePath(std::shared_ptr<VcsTreeItem> item, wxString relativeName)
    {
        this->item = item;
        this->relativeName = relativeName;
    }
};

static int git_status_cb_fn (const char *path, unsigned int statusFlags, void *payload)
{
    std::tuple < std::vector<VcsTreeItemAndRelativePath>*, const LibGit2UpdateFullOp*> *param = (std::tuple < std::vector<VcsTreeItemAndRelativePath>*, const LibGit2UpdateFullOp*> *)payload;
    std::vector<VcsTreeItemAndRelativePath>& tree = *std::get<0>(*param);
    const LibGit2UpdateFullOp* obj  = std::get<1>(*param);

#ifdef TRACE
    fprintf(stderr, "LibGit2::%s:%d path %s\n", __FUNCTION__, __LINE__, path);
#endif
    wxString wxPath = wxString(path);
    auto it = std::find_if(tree.begin(), tree.end(), [&wxPath](const VcsTreeItemAndRelativePath &item)
                                                    {
                                                        return (item.relativeName == wxPath);
                                                    });
    if (it != tree.end())
    {
        fprintf(stderr, "LibGit2::%s:%d path %s present in list. statusFlags  0x%x\n", __FUNCTION__, __LINE__, path, statusFlags);
        obj->m_pendingStates.emplace_back(std::move(it->item), getItemStateFromLibGit2StatusFlag(statusFlags));
        std::iter_swap(it, tree.end() - 1);
        tree.erase(tree.end() - 1);
    }
#ifdef TRACE
    else
    {
        fprintf(stderr, "LibGit2::%s:%d path %s not present in list. statusFlags  %u\n", __FUNCTION__, __LINE__, path, statusFlags);
    }
#endif
    return obj->IsAborted()? 1 : 0;
}

void LibGit2UpdateFullOp::stopExecution()
{
    m_abort = true;
    if (m_executionThread.joinable())
        m_executionThread.join();
}

void LibGit2UpdateFullOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> projectFiles)
{
    fprintf(stderr, "LibGit2::%s:%d Enter. m_VcsRootDir %s proj_files size %zu\n", __FUNCTION__, __LINE__, m_VcsRootDir.ToUTF8().data(),
            projectFiles.size());
    auto executionFn = [this] (std::vector<std::shared_ptr<VcsTreeItem>> projectFiles) mutable
    {
        wxStopWatch sw;
        GitRepo gitRepo(m_VcsRootDir);
        if (gitRepo.m_repo)
        {
            std::vector <VcsTreeItemAndRelativePath> projectFilesAndRelativePaths;
            projectFilesAndRelativePaths.reserve(projectFiles.size());
            for (std::shared_ptr<VcsTreeItem>& item : projectFiles)
            {
                projectFilesAndRelativePaths.emplace_back(std::move(item), item->GetRelativeName(m_VcsRootDir));
            }
            std::tuple < std::vector<VcsTreeItemAndRelativePath>*, const LibGit2UpdateFullOp*> param;
            param = make_tuple(&projectFilesAndRelativePaths, this);
#ifdef TRACE
            fprintf(stderr, "LibGit2::%s:%d before git_status_foreach. workDir %s\n", __FUNCTION__, __LINE__, m_VcsRootDir.ToUTF8().data());
#endif
            git_status_options opts = GIT_STATUS_OPTIONS_INIT;
            opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
            opts.flags = GIT_STATUS_OPT_INCLUDE_IGNORED | GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_INCLUDE_UNMODIFIED;
            git_status_foreach_ext( gitRepo.m_repo, &opts, git_status_cb_fn, &param);
            for (auto &pf : projectFilesAndRelativePaths)
            {
                ItemState itemState;
                wxString fileName = m_VcsRootDir + wxFileName::GetPathSeparator() + pf.relativeName;
                if (wxFileExists(fileName))
                {
                    itemState = Item_Untracked;
                }
                else
                {
                    fprintf(stderr, "LibGit2::::%s:%d[%p] item %s do not exit\n", __FUNCTION__, __LINE__, this, fileName.ToUTF8().data());
                    itemState = Item_UntrackedMissing;
                }
                m_pendingStates.emplace_back(std::move(pf.item), itemState);
            }
        }
        fprintf(stderr, "LibGit2::LibGit2UpdateFullOp[%p] Async git state Update:%d Exit. Took %ld ms\n", this, __LINE__, sw.Time());
        CallAfter(&LibGit2UpdateFullOp::UpdateStates);
    };
    m_abort = true;
    if (m_executionThread.joinable())
        m_executionThread.join();
    m_abort = false;
    m_executionThread = std::thread(executionFn, std::move(projectFiles));;
}

void LibGit2UpdateFullOp::UpdateStates()
{
#ifdef TRACE
    wxStopWatch sw;
#endif
    for (auto &item : m_pendingStates)
    {
        VcsTreeItem *pf = item.m_treeItem.get();
        pf->SetState(item.m_State);
        pf->VisualiseState();
        if (m_abort)
        {
            fprintf(stderr, "LibGit2::%s:%d break as aborted\n", __FUNCTION__, __LINE__);
            break;
        }
    }
#ifdef TRACE
    fprintf(stderr, "LibGit2::LibGit2UpdateFullOp[%p] Update:%d Exit. SetState of %zu items took %ld ms\n", this, __LINE__, m_pendingStates.size(), sw.Time());
#endif
}

/***********************************************************************
 *  Method: LibGit2AddOp::execute
 *  Params: std::vector<VcsTreeItem *> &
 * Returns: void
 * Effects:
 ***********************************************************************/
void LibGit2AddOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> pathList)
{
    wxString pathString;
    GitRepoIndex gitRepoIndex(m_VcsRootDir);
    if (!gitRepoIndex.m_idx)
    {
        fprintf(stderr, "LibGit2::%s:%d gitRepoIndex.m_idx not available\n", __FUNCTION__, __LINE__);
        return;
    }

    for (auto &vcsTreeItem : pathList)
    {
        wxString relativeFilename = vcsTreeItem->GetRelativeName(m_VcsRootDir);
        if (relativeFilename.length() == 0)
        {
            continue;
        }
        int error = git_index_add_bypath(gitRepoIndex.m_idx, relativeFilename.ToUTF8().data());
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_index_add_bypath failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
    }
}

/***********************************************************************
 *  Method: LibGit2CommitOp::execute
 *  Params: std::vector<VcsTreeItem *> &
 * Returns: void
 * Effects:
 ***********************************************************************/
void LibGit2CommitOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> pathList)
{
    wxArrayString itemList;
    LibGit2AddOp::ExecuteImplementation(pathList);
    for (auto &vcsTreeItem : pathList)
    {
        wxString relativeFilename = vcsTreeItem->GetRelativeName(m_VcsRootDir);
        if (relativeFilename.length() == 0)
        {
            continue;
        }

        if (vcsTreeItem->GetState() == Item_Added || vcsTreeItem->GetState() == Item_Modified || vcsTreeItem->GetState() == Item_Removed)
        {
            itemList.Add(relativeFilename);
        }
    }

    if (itemList.size() == 0)
    {
        return;
    }

    wxString msg;
    CommitMsgDialog dlg(Manager::Get()->GetAppWindow(), msg, itemList);

    DumpOutput(itemList);

    if (dlg.ShowModal() == wxID_OK)
    {
        GitRepoIndex gitRepoIndex(m_VcsRootDir);
        git_repository *repo = gitRepoIndex.m_gitRepo.m_repo;
        if (!gitRepoIndex.m_idx)
        {
            fprintf(stderr, "LibGit2::%s:%d gitRepoIndex.m_idx not available\n", __FUNCTION__, __LINE__);
            return;
        }
        git_signature *sig;
        git_oid commit_id;

        int error = git_signature_default(&sig, repo);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_signature_default failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            return;
        }
        git_oid tree_id;
        git_tree *tree;
        error = git_index_write_tree(&tree_id, gitRepoIndex.m_idx);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_index_write_tree failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            git_signature_free(sig);
            return;
        }
        error = git_tree_lookup(&tree, repo, &tree_id);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_tree_lookup failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            git_signature_free(sig);
            return;
        }

        git_oid parent_id;
        error = git_reference_name_to_id(&parent_id, repo, "HEAD");
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_reference_name_to_id failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            git_signature_free(sig);
            git_tree_free(tree);
            return;
        }

        git_commit *parent;
        error = git_commit_lookup(&parent, repo, &parent_id);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_reference_name_to_id failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            git_signature_free(sig);
            git_tree_free(tree);
            return;
        }

        error = git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL, msg.ToUTF8().data(), tree, 1, &parent);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_commit_create failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }

        git_commit_free(parent);
        git_signature_free(sig);
        git_tree_free(tree);
    }
}

/***********************************************************************
 *  Method: LibGit2RemoveOp::execute
 *  Params: std::vector<VcsTreeItem *> &
 * Returns: void
 * Effects:
 ***********************************************************************/
void LibGit2RemoveOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> pathList)
{
    GitRepoIndex gitRepoIndex(m_VcsRootDir);
    if (!gitRepoIndex.m_idx)
    {
        fprintf(stderr, "LibGit2::%s:%d gitRepoIndex.m_idx not available\n", __FUNCTION__, __LINE__);
        return;
    }

    for (auto &vcsTreeItem : pathList)
    {
        wxString relativeFilename = vcsTreeItem->GetRelativeName(m_VcsRootDir);
        if (relativeFilename.length() == 0)
        {
            continue;
        }
        int error = git_index_remove_bypath(gitRepoIndex.m_idx, relativeFilename.ToUTF8().data());
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_index_remove_bypath failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
    }
}

static int diff_aggragator_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *l, void *data)
{
    (void)delta;
    (void)hunk;
    wxString *diff = (wxString *)data;
    if (l->origin == GIT_DIFF_LINE_CONTEXT || l->origin == GIT_DIFF_LINE_ADDITION || l->origin == GIT_DIFF_LINE_DELETION)
        diff->append(l->origin);

    diff->append(l->content, l->content_len);
    return 0;
}

/***********************************************************************
 *  Method: LibGit2DiffOp::ExecuteImplementation
 *  Params: std::vector<VcsTreeItem *> &
 * Returns: void
 * Effects:
 ***********************************************************************/
void LibGit2DiffOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> pathList)
{
    GitRepo gitRepo(m_VcsRootDir);
    if (!gitRepo.m_repo)
    {
        fprintf(stderr, "LibGit2::%s:%d gitRepo.m_repo not available\n", __FUNCTION__, __LINE__);
        return;
    }

    git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
    opts.pathspec.strings = (char **)malloc(sizeof(char *) * pathList.size());
    for (auto &vcsTreeItem : pathList)
    {
        wxString relativeFilename = vcsTreeItem->GetRelativeName(m_VcsRootDir);
        if (relativeFilename.length() == 0)
        {
            continue;
        }
        opts.pathspec.strings[opts.pathspec.count] = strdup(relativeFilename.ToUTF8().data());
        opts.pathspec.count++;
    }
    if (opts.pathspec.count)
    {
        git_diff *diff;
        int error = git_diff_index_to_workdir(&diff, gitRepo.m_repo, NULL, &opts);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_diff_index_to_workdir failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
        else
        {
            wxString diffStr;
            error = git_diff_print(diff, GIT_DIFF_FORMAT_PATCH, diff_aggragator_cb, &diffStr);
            if (0 != error)
            {
                const git_error *e = git_error_last();
                fprintf(stderr, "LibGit2::%s:%d git_diff_print failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            }
            else
            {
                cbEditor *editor = Manager::Get()->GetEditorManager()->New(_("Diff to index.patch"));
                cbStyledTextCtrl *ctrl = editor->GetControl();
                ctrl->SetLexerLanguage(wxT("diff"));
                ctrl->SetText(diffStr);
                editor->SetModified(false);
            }
            git_diff_free(diff);
        }

        for (size_t count = 0; count < opts.pathspec.count; count++)
        {
            free(opts.pathspec.strings[count]);
        }
    }
    else
    {
        fprintf(stderr, "LibGit2::%s:%d no files\n", __FUNCTION__, __LINE__);
    }
    free(opts.pathspec.strings);
}

/***********************************************************************
 *  Method: LibGit2RestoreOp::execute
 *  Params: std::vector<VcsTreeItem *> &
 * Returns: void
 * Effects:
 ***********************************************************************/
void LibGit2RestoreOp::ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>> pathList)
{
    GitRepo gitRepo(m_VcsRootDir);
    if (!gitRepo.m_repo)
    {
        fprintf(stderr, "LibGit2::%s:%d gitRepo.m_repo not available\n", __FUNCTION__, __LINE__);
        return;
    }

    git_checkout_options opts = {GIT_CHECKOUT_OPTIONS_VERSION, GIT_CHECKOUT_FORCE};
    opts.paths.strings = (char **)malloc(sizeof(char *) * pathList.size());
    for (auto &vcsTreeItem : pathList)
    {
        wxString relativeFilename = vcsTreeItem->GetRelativeName(m_VcsRootDir);
        if (relativeFilename.length() == 0)
        {
            continue;
        }
        opts.paths.strings[opts.paths.count] = strdup(relativeFilename.ToUTF8().data());
        opts.paths.count++;
    }
    if (opts.paths.count)
    {
        int error = git_checkout_head(gitRepo.m_repo, &opts);
        if (0 != error)
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_checkout_head failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
        for (size_t count = 0; count < opts.paths.count; count++)
        {
            free(opts.paths.strings[count]);
        }
    }
    else
    {
        fprintf(stderr, "LibGit2::%s:%d no files\n", __FUNCTION__, __LINE__);
    }
    free(opts.paths.strings);
}
