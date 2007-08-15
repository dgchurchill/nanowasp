
#include <stdexcept>
#include <string>
#include <fstream>

#include "CPMFileSys.h"

const char cmd[] = "CPMFileSys";



CPMFileSys::CPMFileSys(const char *name)
{
    if (name == NULL)
        throw std::logic_error("CPMFileSys: name is NULL");

    const char *err = Device_open(&superblock.dev, name, O_RDWR, NULL);
    if (err != NULL)
        throw GeneralError(err);

    cpmReadSuper(&superblock, &root, "microbee");
}


CPMFileSys::~CPMFileSys()
{
    cpmUmount(&superblock);
}


void CPMFileSys::GetDirectory(std::vector<DirEntry> &result)
{
    result.clear();

    struct cpmFile dir;
    struct cpmDirent dirent;
    cpmOpendir(&root, &dir);

    while (cpmReaddir(&dir, &dirent) == 1)
    {
        DirEntry de;

        de.realname = dirent.name;
        if (de.realname != "." && de.realname != "..")
        {
            struct cpmInode file;
            struct cpmStat stats;

            cpmNamei(&root, dirent.name, &file);
            cpmStat(&file, &stats);

            de.user = (dirent.name[0] - '0') * 10 + (dirent.name[1] - '0');
            de.name = dirent.name + 2;  
            de.size = stats.size;

            result.push_back(de);
        }
    }

    cpmClose(&dir);
}


void CPMFileSys::CopyFromCPM(const char *from, const char *to)
{
    struct cpmInode ino;
    struct cpmFile file;

    if (cpmNamei(&root, from, &ino) == -1)
        throw FileNotFound(from);

    cpmOpen(&ino, &file, O_RDONLY);

    std::ofstream outf(to, std::ios::out | std::ios::binary | std::ios::trunc);
    if (outf.fail())
        throw NotWritable(to);


    int count;
    char buf[4096];

    while ((count = cpmRead(&file, buf, sizeof(buf))) != 0)
    {
        for (int i = 0; i < count; ++i)
            outf << buf[i];
    }

    outf.close();
    cpmClose(&file);
}


void CPMFileSys::CopyToCPM(const char *from, const char *to)
{
    std::ifstream inf(from, std::ios::in | std::ios::binary);
    if (inf.fail())
        throw FileNotFound(from);

    struct cpmInode ino;
    if (cpmCreat(&root, to, &ino, 0666) == -1)
        throw NotWritable(to);

    struct cpmFile file;
    cpmOpen(&ino, &file, O_WRONLY);

    char buf[4096];

    while (inf.good())
    {
        int i = 0;

        while (i < sizeof(buf) && inf.get(buf[i]))
            ++i;

        if (i > 0)
        {
            if (cpmWrite(&file, buf, i) != i)
            {
                cpmClose(&file);
                cpmUnlink(&root, to);  // Delete the partially written file
                throw NotWritable(to);
            }
        }
    }

    cpmClose(&file);
    cpmSync(&superblock);
    inf.close();
}


void CPMFileSys::Delete(const char *file)
{
    if (cpmUnlink(&root, file) == -1)
        throw GeneralError(boo);
}


void CPMFileSys::Rename(const char *from, const char *to)
{
    if (cpmRename(&root, from, to) == -1)
        throw GeneralError(boo);
}


void CPMFileSys::GetStat(struct CPMFSStat &stat)
{
    struct cpmStatFS buf;

    cpmStatFS(&root, &buf);
    stat.size = buf.f_bsize * buf.f_blocks;
    stat.free = buf.f_bsize * buf.f_bfree;
}


std::string CPMFileSys::RealName(int user, const char *name)
{
    std::string result;

    if (user < 0 || user > 99)
        user = 0;

    result = '0' + (char)user / 10;
    result += '0' + (char)user % 10;
    result += name;

    return result;
}