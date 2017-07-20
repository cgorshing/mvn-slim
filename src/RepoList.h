#ifndef REPOLIST_H
#define REPOLIST_H

class RepoList {
public:
    RepoList();
    RepoList(const RepoList& orig);
    virtual ~RepoList();
    
    virtual bool addRepo();
private:

};

#endif /* REPOLIST_H */

