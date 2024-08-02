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
#include "vcsfactory.h"
#ifdef USE_LIBGIT2
#include "git_libgit2.h"
#else
#include "git.h"
#endif
#include "icommandexecuter.h"

/*static*/ IVersionControlSystem* VcsFactory::GetVcs(const wxString& ProjectPath,
                                                     ICommandExecuter& shellUtils)
{
#ifdef USE_LIBGIT2
    if(LibGit2::IsGitRepo(ProjectPath, shellUtils))
    {
        return new LibGit2(ProjectPath, shellUtils);
    }
#else
    if(git::IsGitRepo(ProjectPath, shellUtils))
    {
        return new git(ProjectPath, shellUtils);
    }

#endif
    return 0;
}
