#ifndef FSDISK_H
#define FSDISK_H

#include "FsFile.h"
#include "FileDescriptor.h"
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <cstring>
#include <string>
#include <cmath>
#include <fcntl.h>
#include <cstdlib>

using namespace std;

#define DISK_SIZE 256
#define ASCII_DIFF 48
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"



class fsDisk {
    FILE *sim_disk_fd;
    bool is_formated;
    int block_size;
    int maxFileSize;
    int freeBlocks;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int bitVectorSize{};
    int *bitVector{};

    // Structure that links the file name to its FsFile
    vector<FileDescriptor *> MainDir;


    //  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    map<int, FileDescriptor *> OpenFileDescriptors;

public:
    fsDisk();
    ~fsDisk();
    void listAll();
    void fsFormat(int blockSize = 4);
    int CreateFile(string fileName);
    int OpenFile(string fileName);
    string CloseFile(int fd);
    int WriteToFile(int fd, char *buf, int len);
    int ReadFromFile(int fd, char *buf, int len);
    int DelFile(string fileName);
};


#endif //FSDISK_H
