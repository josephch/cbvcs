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

#include <logmanager.h>
#include "VcsTreeItem.h"

VcsTreeItem::VcsTreeItem(wxString Name, ItemState State) :
    m_Name(Name),
    m_State(State)
{
    //ctor
}

VcsTreeItem::~VcsTreeItem()
{
    //dtor
}

const wxString VcsTreeItem::GetRelativeName(const wxString& RootPath) const
{
    wxString relativeName;

    if(m_Name.StartsWith(RootPath, &relativeName))
    {
        // strip first '/'
        if (wxFileName::IsPathSeparator(relativeName.at(0)))
        {
            relativeName = relativeName(1, relativeName.length());
        }
    }

    return relativeName;
}
