#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H
#include "FsFile.h"
#include <string>

using namespace std;


class FileDescriptor {
    string file_name;
    FsFile *fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile *fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    FsFile* getFsFile() {
        return this->fs_file;
    }

    string getFileName() {
        return file_name;
    }

    bool getInUse() {
        return this->inUse;
    }
    void setInUse(bool var) {
        this->inUse = var;
    }

};



#endif //FILEDESCRIPTOR_H
