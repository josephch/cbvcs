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
#ifndef VCSFILEOP_H
#define VCSFILEOP_H

#include <memory>
#include <vector>
#include <wx/string.h>
#include "icommandexecuter.h"

class VcsTreeItem;

class VcsFileOp
{
    public:
        VcsFileOp(const wxString& vcsRootDir, ICommandExecuter& shellUtils) :
            m_VcsRootDir(vcsRootDir),
            m_ShellUtils(shellUtils)
            {}

        virtual ~VcsFileOp() {}
        void execute(std::vector<std::shared_ptr<VcsTreeItem>>);

    protected:
        const wxString& m_VcsRootDir;
        ICommandExecuter& m_ShellUtils;

    private:
        virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>) = 0;
};

#endif // VCSFILEOP_H
