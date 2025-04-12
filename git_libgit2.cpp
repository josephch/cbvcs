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

#include "git_libgit2.h"
#include "git_libgit2_wrapper.h"
#include "icommandexecuter.h"
#include <git2.h>
#include <manager.h>
#include <wx/dir.h>
#include <wx/string.h>

LibGit2::LibGit2(const wxString &project, ICommandExecuter &cmdExecutor, wxString workDirectory)
    : IVersionControlSystem(project, &m_GitUpdate, &m_GitAdd, &m_GitRemove, &m_GitCommit, &m_GitDiff, &m_GitRestore, &m_GitUpdateFull),
      m_workDirectory(std::move(workDirectory)),
      m_CmdExecutor(cmdExecutor),
      m_GitUpdate(*this, m_GitRoot, m_CmdExecutor),
      m_GitAdd(*this, m_GitRoot, m_CmdExecutor),
      m_GitRemove(*this, m_GitRoot, m_CmdExecutor),
      m_GitCommit(*this, m_GitRoot, m_CmdExecutor),
      m_GitDiff(*this, m_GitRoot, m_CmdExecutor),
      m_GitRestore(*this, m_GitRoot, m_CmdExecutor),
      m_GitUpdateFull(*this, m_GitRoot, m_CmdExecutor)
{
    git_libgit2_init();
    m_GitRoot = QueryRoot(m_workDirectory.ToUTF8().data());
}

LibGit2::~LibGit2() { git_libgit2_shutdown(); }

wxString LibGit2::QueryRoot(const char *gitWorkDirInProject)
{
    git_buf root = {0};
    wxString ret;
    git_libgit2_init();
    int error = git_repository_discover(&root, gitWorkDirInProject, 0, NULL);
    if (!error)
    {
        git_repository *repo;
        error = git_repository_open(&repo, gitWorkDirInProject);
        if (0 == error)
        {
            const char *workDir = git_repository_workdir(repo);
            wxFileName workDirFileName(workDir);
            wxFileName relativePath(gitWorkDirInProject);
            if (workDirFileName != relativePath)
            {
                fprintf(stderr, "LibGit2::%s:%d workdir [%s] and workdir of project [%s] not same\n", __FUNCTION__, __LINE__, workDir,
                        gitWorkDirInProject);
                if (workDirFileName.ResolveLink() == relativePath.ResolveLink())
                {
                    ret = wxString(gitWorkDirInProject);
                }
                else
                {
                    fprintf(stderr, "LibGit2::%s:%d workdir [%s] and realpath of workdir of project [%s] not same\n", __FUNCTION__, __LINE__, workDir,
                            gitWorkDirInProject);
                    ret = wxString(workDir);
                }
            }
            else
            {
                ret = wxString(gitWorkDirInProject);
            }
            git_repository_free(repo);
        }
        else
        {
            const git_error *e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_repository_open failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
    }
    else
    {
        const git_error *e = git_error_last();
        fprintf(stderr, "LibGit2::%s:%d git_repository_discover failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
    }
    git_buf_free(&root);
    git_libgit2_shutdown();
    fprintf(stderr, "LibGit2::%s:%d return root : %s\n", __FUNCTION__, __LINE__, ret.ToUTF8().data());
    return ret;
}

/*static*/ bool LibGit2::IsGitRepo(const wxString &project, ICommandExecuter &shellUtils, wxString *workDirectory)
{
    bool ret;
    git_repository *repo;
    git_libgit2_init();
    wxString projectPath = wxPathOnly(project);
    git_buf root = {0};
    int error = git_repository_discover(&root, projectPath, 0, NULL);
    if (!error)
    {
        fprintf(stderr, "LibGit2::%s:%d project file's path is repo\n", __FUNCTION__, __LINE__);
        ret = true;
        if (workDirectory)
        {
            *workDirectory = root.ptr;
        }
    }
    else
    {
        fprintf(stderr, "LibGit2::%s:%d projectPath %s not workdir\n", __FUNCTION__, __LINE__, projectPath.ToUTF8().data());
        ret = false;
        wxString subDirPath;
        wxDir dir(projectPath);
        if (!dir.IsOpened())
        {
            wxLogError("Cannot open directory '%s'.", projectPath);
        }
        else
        {
            bool cont = dir.GetFirst(&subDirPath, wxEmptyString, wxDIR_DIRS);
            while (cont)
            {
                wxString subDirFullPath = projectPath + wxFILE_SEP_PATH + subDirPath;
                if (0 == git_repository_open(&repo, subDirFullPath))
                {
                    fprintf(stderr, "LibGit2::%s:%d subDirFullPath %s is workdir\n", __FUNCTION__, __LINE__, subDirFullPath.ToUTF8().data());
                    ret = true;
                    if (workDirectory)
                    {
                        *workDirectory = std::move(subDirFullPath);
                    }
                    git_repository_free(repo);
                    break;
                }
                else
                {
                    fprintf(stderr, "LibGit2::%s:%d subDirFullPath %s not workdir\n", __FUNCTION__, __LINE__, subDirFullPath.ToUTF8().data());
                }
                cont = dir.GetNext(&subDirPath);
            }
        }
    }
    git_buf_free(&root);
    git_libgit2_shutdown();
    return ret;
}

wxString LibGit2::GetBranch()
{
    GitRepo gitRepo(m_GitRoot);
    wxString branch;
    git_repository* repo = gitRepo.m_repo;
    if (!repo)
    {
        fprintf(stderr, "LibGit2::%s:%d gitRepo.m_repo not available\n", __FUNCTION__, __LINE__);
        return branch;
    }
    git_reference* head = nullptr;
    int error = git_repository_head(&head, repo);
    if (error == GIT_EUNBORNBRANCH)
    {
        fprintf(stderr, "LibGit2::%s:%d Repository has no commits (unborn branch).\n", __FUNCTION__, __LINE__);
    }
    else if (error < 0)
    {
        const git_error* e = git_error_last();
        fprintf(stderr, "LibGit2::%s:%d Failed to get HEAD: %s\n", __FUNCTION__, __LINE__,
                e ? e->message : "unknown error");
    }
    else if (git_reference_is_branch(head))
    {
        const char* branch_name = nullptr;
        git_branch_name(&branch_name, head);
        branch = branch_name;
        fprintf(stderr, "LibGit2::%s:%d Current branch: %s\n", __FUNCTION__, __LINE__, branch_name);
    }
    else
    {
        fprintf(stderr, "LibGit2::%s:%d Detached HEAD state.\n", __FUNCTION__, __LINE__);
    }

    git_reference_free(head);
    return branch;
}
