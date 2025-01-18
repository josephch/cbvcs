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
#ifndef IVERSIONCONTROLSYSTEM_H
#define IVERSIONCONTROLSYSTEM_H

#include <globals.h>
#include <vector>
#include <map>

class wxString;
class ProjectFile;

#include "VcsFileOp.h"

class IVersionControlSystem
{
    public:
        IVersionControlSystem(const wxString& project,
                              VcsFileOp* update,
                              VcsFileOp* add,
                              VcsFileOp* remove,
                              VcsFileOp* commit,
                              VcsFileOp* diff,
                              VcsFileOp* restore,
                              VcsFileOp* updateFull);
        virtual ~IVersionControlSystem();

        VcsFileOp* UpdateOp;
        VcsFileOp* AddOp;
        VcsFileOp* RemoveOp;
        virtual bool move(std::vector<VcsTreeItem*>&) = 0;
        VcsFileOp* CommitOp;
        VcsFileOp* DiffOp;
        VcsFileOp* RestoreOp;
        VcsFileOp* UpdateFullOp;

    protected:
        const wxString& m_project;
    private:
};

#endif // IVERSIONCONTROLSYSTEM_H
