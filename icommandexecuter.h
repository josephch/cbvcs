#ifndef ICOMMANDEXECUTER_H
#define ICOMMANDEXECUTER_H

#include "copyprotector.h"

class wxString;
class wxArrayString;

class ICommandExecuter : private CopyProtector
{
    public:
        /** Default constructor */
        ICommandExecuter() {}
        /** Default destructor */
        virtual ~ICommandExecuter() {}
        virtual bool execute(const wxString& shellCommand,
                             wxArrayString& outCapture,
                             wxArrayString&  errCapture) const = 0;
        virtual void pushd(const wxString& newDir) = 0;
        virtual void popd() = 0;

    protected:
    private:
};

#endif // ICOMMANDEXECUTER_H
