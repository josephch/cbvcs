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
#ifndef LibGit2_H
#define LibGit2_H

#include "IVersionControlSystem.h"
#include "git_libgit2_ops.h"

class wxArrayString;

class LibGit2 : public IVersionControlSystem
{
  public:
    static bool IsGitRepo(const wxString &project, ICommandExecuter &, wxString *workDirectory);
    LibGit2(const wxString &project, ICommandExecuter &cmdExecutor, wxString workDirectory);
    virtual ~LibGit2();

    virtual bool move(std::vector<VcsTreeItem *> &) override { return false; }
    wxString GetBranch() override;

  protected:
    wxString m_workDirectory;
    wxString m_GitRoot;

  private:
    ICommandExecuter &m_CmdExecutor;
    LibGit2UpdateOp m_GitUpdate;
    LibGit2AddOp m_GitAdd;
    LibGit2RemoveOp m_GitRemove;
    LibGit2CommitOp m_GitCommit;
    LibGit2DiffOp m_GitDiff;
    LibGit2RestoreOp m_GitRestore;
    LibGit2UpdateFullOp m_GitUpdateFull;

    wxString QueryRoot(const char *);
};

#endif // GIT_H
