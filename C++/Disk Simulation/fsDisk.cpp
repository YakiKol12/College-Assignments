#include "fsDisk.h"


//CONSTRUCTOR
fsDisk::fsDisk() {
    sim_disk_fd = fopen(DISK_SIM_FILE, "w+");
    assert(sim_disk_fd);

    for (int i = 0; i < DISK_SIZE; i++) {
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = fwrite("\0", 1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }

    fflush(sim_disk_fd);
    is_formated = false;
    this->block_size = 0;
    this->maxFileSize = 0;
    this->freeBlocks = 0;
}

//DESTRUCTOR
fsDisk::~fsDisk() {
    for(auto & i : this->MainDir) {
        delete i->getFsFile();
        delete i;
    }
    this->MainDir.clear();
    this->OpenFileDescriptors.clear();
    delete[] this->bitVector;
    fclose(this->sim_disk_fd);
}

//This method prints out a list of created files in the disk and prints the content of the disk
void fsDisk:: listAll() {
    int i = 0;

    for (i = 0; i < this->MainDir.size(); i++) {
        cout << "index: " << i << ": FileName: " << this->MainDir[i]->getFileName() <<  " , isInUse: " << this->MainDir[i]->getInUse() << endl;
    }

    char bufy;
    cout << "Disk content: '";
    for (i = 0; i < DISK_SIZE; i++) {
        cout << "(";
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = fread(&bufy, 1, 1, sim_disk_fd);
        cout << bufy;
        cout << ")";
    }
    cout << "'" << endl;
}

/**
 * This method deletes the content of the disk and determines its new block size
 * @param blockSize
 */
void fsDisk:: fsFormat(int blockSize) {
    this->block_size = blockSize;
    this->maxFileSize = blockSize*blockSize;
    this->freeBlocks = floor(DISK_SIZE / blockSize);
    this->bitVectorSize = floor(DISK_SIZE / blockSize);

    //reset the bit vector
    delete[] this->bitVector;
    this->bitVector = new int[this->bitVectorSize];
    for (int i = 0; i < bitVectorSize; i++)
        this->bitVector[i] = 0;

    //reset disk content
    for (int i = 0; i < DISK_SIZE; i++) {
        int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
        ret_val = fwrite("\0", 1, 1, sim_disk_fd);
        assert(ret_val == 1);
    }
    fflush(sim_disk_fd);

    for(auto & i : this->MainDir) {
        delete i->getFsFile();
        delete i;
    }
    this->MainDir.clear();
    this->OpenFileDescriptors.clear();

    this->is_formated = true;
    cout<<"FORMAT DISK: number of blocks "<< this->bitVectorSize <<endl;
}

/**
 * This method creates a new file, allocate a new indexBlock for it
 * @param fileName
 * @return int - the file descriptor of the file
 */
int fsDisk:: CreateFile(string fileName) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        return -1;
    }

    for (auto & entry : this->MainDir) {
        if(entry->getFileName() == fileName) {
            cout<<"This file already exists"<<endl;
            return -1;
        }
    }

    //creating file
    auto *fs = new FsFile(this->block_size);
    auto *fd = new FileDescriptor(fileName, fs);

    this->MainDir.push_back(fd);

    //inserting to file descriptor table
    int i = 0;
    while(true) {
        if (this->OpenFileDescriptors.find(i) == this->OpenFileDescriptors.end()) {
            this->OpenFileDescriptors.insert({i, fd});
            break;
        }
        i++;
    }
    return i;
}

/**
 * This method search for a file named @fileName, if it is closed, open it and attach a new file descriptor to it
 * @param fileName
 * @return int - file descriptor of the file
 */
int fsDisk:: OpenFile(string fileName) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        return -1;
    }


    int i;
    for(i = 0; i < this->MainDir.size(); i++) {
        if(this->MainDir[i]->getFileName() == fileName)
            break;
    }
    if(i == this->MainDir.size()) {
        cout<<"File not exist"<<endl;
        return -1;
    }


    for (auto entry: this->OpenFileDescriptors) {
        if (entry.second->getFileName() == fileName) {
            cout << "file already open" << endl;
            return -1;
        }
    }

    this->MainDir[i]->setInUse(true);

    int fd;
    for (fd = 0; fd < this->bitVectorSize; fd++) {
        if (this->OpenFileDescriptors.find(fd) == this->OpenFileDescriptors.end()) {
            this->OpenFileDescriptors.insert({fd, this->MainDir[i]});
            break;
        }
    }
    return fd;
}

/**
 * This method search for a file using its file descriptor, if it is open close it and release its fd
 * @param fd
 * @return string - file name on success, "-1" if failed
 */
string fsDisk:: CloseFile(int fd) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        return "-1";
    }

    auto search = this->OpenFileDescriptors.find(fd);
    if(search == this->OpenFileDescriptors.end()) {
        cout<< "No such file is open" <<endl;
        return "-1";
    }

    string fileName = search->second->getFileName();
    search->second->setInUse(false);
    this->OpenFileDescriptors.erase(fd);
    return fileName;
}

/**
 * This method write data to a specific file on the disk.
 * @param fd  - file descriptor of the file to write to.
 * @param buf - string to write.
 * @param len - length of buf, how many chars to write.
 * @return int - 1 if succeeded to write, -1 if failed.
 */
int fsDisk:: WriteToFile(int fd, char *buf, int len) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        return -1;
    }

    //check if file is open
    auto search = this->OpenFileDescriptors.find(fd);
    if(search == this->OpenFileDescriptors.end()) {
        cout<< "File is not open" <<endl;
        return -1;
    }

    FsFile* fileToWrite = search->second->getFsFile();
    int fileSize = fileToWrite->getFileSize();
    int indexBlockFlag = 0;

    if(fileToWrite->getIndexBlock() == -1) {
        indexBlockFlag = 1;
    }

    // check for memory in the file
    if(len + fileSize > maxFileSize) {
        cout<< "Not enough memory in file" <<endl;
        return -1;
    }

    // check if I have enough free memory for the string in the disk
    if(this->freeBlocks * this->block_size < len - fileToWrite->getRemainingMemoryInBlock() + (indexBlockFlag * this->block_size)) {
        cout<< "Not enough memory in disk" <<endl;
        return -1;
    }

    //allocate index block
    if(indexBlockFlag == 1) {
        //search for a free block in disk
        for (int i = 0; i < this->bitVectorSize; i++) {
            if (this->bitVector[i] == 0) {
                fileToWrite->setIndexBlock(i);
                this->bitVector[i] = 1;
                this->freeBlocks--;
                break;
            }
        }
    }


    // in here we know that we have enough memory, both in disk and in file, to write to the file
    int indexBlock = fileToWrite->getIndexBlock();
    int blockToWrite;
    int charToWrite = 0;


    while(len > 0) {
        //allocate new block, if and when needed
        if(fileToWrite->getRemainingMemoryInBlock() == 0) {
            for (int i = 0; i < this->bitVectorSize; i++) {
                if (this->bitVector[i] == 0) { // found empty block
                    bitVector[i] = 1;
                    fseek(this->sim_disk_fd, (indexBlock * this->block_size) + fileToWrite->getBlocksInUse(), SEEK_SET);
                    putc(i + ASCII_DIFF, this->sim_disk_fd);
                    break;
                }
            }
            fileToWrite->setBlockInUse(fileToWrite->getBlocksInUse() + 1);
            this->freeBlocks--;
        }

        //move cursor to the next writing spot
        fseek(this->sim_disk_fd, (indexBlock * this->block_size) + fileToWrite->getBlocksInUse() - 1 , SEEK_SET);
        blockToWrite = getc(this->sim_disk_fd) - ASCII_DIFF;
        fseek(this->sim_disk_fd, (blockToWrite * this->block_size) + (fileToWrite->getFileSize() % this->block_size), SEEK_SET);

        //write to the file
        while(fileToWrite->getRemainingMemoryInBlock() > 0 && len > 0) {
            fputc(buf[charToWrite++], this->sim_disk_fd);
            fileToWrite->setFileSize(1);
            len--;
        }
    }
    return 1;
}

/**
 * This method reads @len chars from the file tagged with @fd
 * @param fd
 * @param buf
 * @param len
 * @return int - 1 if succeeded, -1 if failed
 */
int fsDisk:: ReadFromFile(int fd, char *buf, int len) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        buf[0] = '\0';
        return -1;
    }

    //check if file is open
    auto search = this->OpenFileDescriptors.find(fd);
    if(search == this->OpenFileDescriptors.end()) {
        cout<< "File is not open" <<endl;
        buf[0] = '\0';
        return -1;
    }

    FsFile* fileToRead = search->second->getFsFile();
    if(len > fileToRead->getFileSize()) {
        cout<< "File is too small" <<endl;
        buf[0] = '\0';
        return -1;
    }

    int indexBlock = fileToRead->getIndexBlock();
    int blockToRead = 0;
    int offSet = 0;
    int charsRead = 0;

    while(len > 0) {
        fseek(this->sim_disk_fd, (indexBlock * this->block_size) + offSet, SEEK_SET);
        blockToRead = getc(this->sim_disk_fd) - ASCII_DIFF;
        fseek(this->sim_disk_fd, blockToRead * this->block_size, SEEK_SET);
        for(int i = 0; i < this->block_size && len > 0; i++, len--) {
            buf[charsRead++] = fgetc(this->sim_disk_fd);
        }
        offSet++;
    }
    buf[charsRead] = '\0';

    return 1;
}

/**
 * This method deletes a file from the disk
 * @param fileName
 * @return
 */
int fsDisk:: DelFile(string fileName) {
    if(!this->is_formated) {
        cout<<"Disk is not formatted"<<endl;
        return -1;
    }

    // first, make sure the file exists
    int fileIndex;
    for(fileIndex = 0; fileIndex < this->MainDir.size(); fileIndex++) {
        if(this->MainDir[fileIndex]->getFileName() == fileName) {
            break;
        }
    }
    if(fileIndex == this->MainDir.size()) {
        cout<<"File not exist"<<endl;
        return -1;
    }

    //if file is open, return error
    for (auto entry: this->OpenFileDescriptors) {
        if (entry.second->getFileName() == fileName) {
            cout<<"File is open, cannot delete"<<endl;
            return -1;
        }
    }

    FileDescriptor* fileDes = this->MainDir[fileIndex];
    FsFile* temp = fileDes->getFsFile();
    int indexBlock = temp->getIndexBlock();
    int deleteBlock = 0;

    for(int i = 0; i < this->block_size; i++) {
        fseek(this->sim_disk_fd, (indexBlock * this->block_size) + i, SEEK_SET);
        deleteBlock = getc(this->sim_disk_fd);
        if(deleteBlock != 0) {
            deleteBlock -= ASCII_DIFF;
            fseek(this->sim_disk_fd, deleteBlock * this->block_size, SEEK_SET);
            for(int j = 0; j < this->block_size; j++) {
                fwrite("\0", 1, 1, sim_disk_fd);
            }
            this->bitVector[deleteBlock] = 0;
            this->freeBlocks++;
        }
    }

    if(indexBlock >= 0) {
        fseek(this->sim_disk_fd, indexBlock * this->block_size, SEEK_SET);
        for(int j = 0; j < this->block_size; j++) {
            fwrite("\0", 1, 1, sim_disk_fd);
        }
        this->bitVector[indexBlock] = 0;
        this->freeBlocks++;
    }

    delete temp;
    delete fileDes;
    this->MainDir.erase(this->MainDir.begin() + fileIndex);
    return 1;
}

