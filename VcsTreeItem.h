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
#ifndef VCSTREEITEM_H
#define VCSTREEITEM_H

#include <wx/string.h>
#include <globals.h>

enum ItemState
{
    Item_Untracked = fvsNormal,
    Item_UntrackedMissing = fvsMissing,
    Item_Added = fvsVcAdded,
    Item_Conflicted = fvsVcConflict,
    Item_Modified = fvsVcModified,
    Item_Removed = fvsVcNonControlled,
    Item_Missing = fvsVcMissing,
    Item_UpToDate = fvsVcUpToDate
};

class VcsTreeItem
{
    public:
        VcsTreeItem(wxString Name, ItemState State);
        virtual ~VcsTreeItem();
        ItemState GetState() const { return m_State; }
        void SetState(ItemState val) { m_State = val; }
        const wxString& GetName() { return m_Name; }
        wxString GetRelativeName(const wxString& RootPath) const;
        void SetName(wxString val) { m_Name = val; }
        virtual void VisualiseState() const = 0;

    private:
        wxString m_Name;
        ItemState m_State;
};

#endif // VCSTREEITEM_H
