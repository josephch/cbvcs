#ifndef LibGit2_OPS_H
#define LibGit2_OPS_H

#include "VcsFileOp.h"
#include "VcsTreeItem.h"
#include <atomic>
#include <thread>

class LibGit2;

class LibGit2_Op : public VcsFileOp
{
  public:
    LibGit2_Op(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : VcsFileOp(vcsRootDir, shellUtils), m_vcs(vcs) {}

  protected:
    void DumpOutput(const wxArrayString &array) const
    {
        fprintf(stderr, "LibGit2::%s:%d array size %zu\n", __FUNCTION__, __LINE__, array.size());
        for (size_t i = 0; i < array.size(); ++i)
        {
            fprintf(stderr, "LibGit2::%s:%d array[%zu] = %s\n", __FUNCTION__, __LINE__, i, array[i].ToUTF8().data());
        }
    };
    LibGit2 &m_vcs;

  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>) = 0;
};

class ICommandExecuter;
enum ItemState;

class LibGit2UpdateOp : public LibGit2_Op
{
  public:
    LibGit2UpdateOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils);
  protected:
    void UpdateItem(VcsTreeItem *pf, const class GitRepo& gitRepo) const;
  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2AddOp : public LibGit2_Op
{
  public:
    LibGit2AddOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2_Op(vcs, vcsRootDir, shellUtils) {}

  protected:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2CommitOp : public LibGit2AddOp
{
  public:
    LibGit2CommitOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2AddOp(vcs, vcsRootDir, shellUtils) {}

  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2DiffOp : public LibGit2_Op
{
  public:
    LibGit2DiffOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2_Op(vcs, vcsRootDir, shellUtils) {}

  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2RemoveOp : public LibGit2_Op
{
  public:
    LibGit2RemoveOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2_Op(vcs, vcsRootDir, shellUtils) {}

  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2RevertOp : public LibGit2_Op
{
  public:
    LibGit2RevertOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils) : LibGit2_Op(vcs, vcsRootDir, shellUtils) {}

  private:
    virtual void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>);
};

class LibGit2UpdateFullOp : public LibGit2UpdateOp, public wxEvtHandler
{
  public:
    LibGit2UpdateFullOp(LibGit2 &vcs, const wxString &vcsRootDir, ICommandExecuter &shellUtils);
    bool IsAborted() const{ return m_abort;};
    ~LibGit2UpdateFullOp()
    {
        m_abort = true;
        if (m_executionThread.joinable())
            m_executionThread.join();
    }
    struct ItemStateValue
    {
        std::shared_ptr<VcsTreeItem> m_treeItem;
        ItemState m_State;
        ItemStateValue(std::shared_ptr<VcsTreeItem> treeItem, ItemState state) : m_treeItem(treeItem), m_State(state){}
    };
    mutable std::vector<ItemStateValue> m_pendingStates;

  private:
    void ExecuteImplementation(std::vector<std::shared_ptr<VcsTreeItem>>) override;
    void UpdateStates();
    mutable std::thread m_executionThread;
    std::atomic_bool m_abort = {false};
};

#endif // LibGit2_OPS_H
