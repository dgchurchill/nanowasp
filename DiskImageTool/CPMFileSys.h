
#ifndef CPMFILESYS_H
#define CPMFILESYS_H

#include "cpmfs.h"
#include <vector>


class CPMFileSys
{
public:
    struct DirEntry
    {
        std::string realname;
        int user;
        std::string name;
        size_t size;
    };


    CPMFileSys(const char *name);
    ~CPMFileSys();


    void GetDirectory(std::vector<DirEntry> &result);
    void CopyFromCPM(const char *from, const char *to);
    void CopyToCPM(const char *from, const char *to);

    std::string RealName(int user, const char *name);


private:
    struct cpmSuperBlock superblock;
    struct cpmInode root;
};


#endif // CPMFILESYS_H