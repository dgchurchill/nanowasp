
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

    struct CPMFSStat
    {
          long size;
          long free;
    };


    CPMFileSys(const char *name);
    ~CPMFileSys();


    void GetDirectory(std::vector<DirEntry> &result);
    void CopyFromCPM(const char *from, const char *to);
    void CopyToCPM(const char *from, const char *to);
    void Delete(const char *file);
    void Rename(const char *from, const char *to);
    bool Exists(const char *file);
    void GetStat(struct CPMFSStat &stat);
    bool IsReadOnly();

    std::string RealName(int user, const char *name);


    class GeneralError : public std::runtime_error
    {
    public:
        explicit GeneralError(const std::string &detail) : std::runtime_error(detail) {}
    };

    class FileNotFound : public GeneralError
    {
    public:
        explicit FileNotFound(const std::string &name) : GeneralError(name) {}
    };

    class NotWritable : public GeneralError
    {
    public:
        explicit NotWritable(const std::string &name) : GeneralError(name) {}
    };


private:
    struct cpmSuperBlock superblock;
    struct cpmInode root;
};


#endif // CPMFILESYS_H