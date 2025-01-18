#ifndef SHELLUTILIMPL_H
#define SHELLUTILIMPL_H

#include <wx/string.h>
#include "icommandexecuter.h"

class ShellUtilImpl : public ICommandExecuter
{
    public:
        /** Default constructor */
        ShellUtilImpl();
        /** Default destructor */
        virtual ~ShellUtilImpl();

        virtual bool execute(const wxString& shellCommand,
                             wxArrayString& outCapture,
                             wxArrayString&  errCapture) const;
        virtual void pushd(const wxString& newDir);
        virtual void popd();
    protected:
    private:
        wxString m_Cwd;
};

#endif // SHELLUTILIMPL_H
