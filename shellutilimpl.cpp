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
#include <wx/utils.h>
#include "shellutilimpl.h"

ShellUtilImpl::ShellUtilImpl()
{
    //ctor
}

ShellUtilImpl::~ShellUtilImpl()
{
    //dtor
}

bool ShellUtilImpl::execute(const wxString& shellCommand,
                            wxArrayString& outCapture,
                            wxArrayString&  errCapture) const
{
    const long pid = wxExecute(shellCommand, outCapture, errCapture, wxEXEC_SYNC);

    if( pid == -1 )
    {
        return false;
    }

    return true;
}

void ShellUtilImpl::pushd(const wxString& newDir)
{
    m_Cwd = wxGetCwd();
    wxSetWorkingDirectory(newDir);
    return ;
}

void ShellUtilImpl::popd()
{
    wxSetWorkingDirectory(m_Cwd);
    return ;
}
