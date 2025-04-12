#ifndef GIT_LIBGIT2_WRAPPER_H_INCLUDED
#define GIT_LIBGIT2_WRAPPER_H_INCLUDED

#include <git2.h>

class GitRepo
{
  public:
    git_repository* m_repo{nullptr};

    GitRepo(const wxString& workDir)
    {
        int error = git_repository_open(&m_repo, workDir.ToUTF8().data());
        if (0 != error)
        {
            const git_error* e = git_error_last();
            fprintf(stderr, "LibGit2::%s:%d git_repository_open failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
        }
    }
    ~GitRepo()
    {
        if (m_repo)
        {
            git_repository_free(m_repo);
        }
    }
};

class GitRepoIndex
{
  public:
    git_index* m_idx{nullptr};
    GitRepo m_gitRepo;

    GitRepoIndex(const wxString& workDir) : m_gitRepo(workDir)
    {
        if (m_gitRepo.m_repo)
        {
            int error = git_repository_index(&m_idx, m_gitRepo.m_repo);
            if (0 != error)
            {
                const git_error* e = git_error_last();
                fprintf(stderr, "LibGit2::%s:%d git_repository_index failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            }
            error = git_index_read(m_idx, true);
            if (0 != error)
            {
                const git_error* e = git_error_last();
                fprintf(stderr, "LibGit2::%s:%d git_index_read failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            }
        }
        else
        {
            fprintf(stderr, "LibGit2::%s:%d gitRepo.m_repo not available\n", __FUNCTION__, __LINE__);
        }
    }

    ~GitRepoIndex()
    {
        if (m_idx)
        {
            int error = git_index_write(m_idx);
            if (0 != error)
            {
                const git_error* e = git_error_last();
                fprintf(stderr, "LibGit2::%s:%d git_index_read failed : %d/%d: %s\n", __FUNCTION__, __LINE__, error, e->klass, e->message);
            }
            git_index_free(m_idx);
        }
    }
};

#endif // GIT_LIBGIT2_WRAPPER_H_INCLUDED
